/**
 * \brief  User interface to genode Nic session
 * \author Stefan Kalkowski
 * \author Alice Domage
 * \date   2024-01-30
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 * Copyright (C) 2024 gapfruit ag
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_user/init.h>

#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/sched/task.h>
#include <linux/rtnetlink.h>
#include <genode_c_api/uplink.h>

struct task_struct *user_task_struct_ptr;  /* used by 'Device' for lx_emul_task_unblock */

static struct genode_uplink *dev_genode_uplink(struct net_device *dev)
{
	return (struct genode_uplink *)dev->ifalias;
}


struct genode_uplink_rx_context
{
	struct net_device *dev;
};


struct genode_uplink_tx_packet_context
{
	struct sk_buff *skb;
};

static unsigned long uplink_tx_packet_content(struct genode_uplink_tx_packet_context *ctx,
                                              char *dst, unsigned long dst_len)
{
	struct sk_buff * const skb = ctx->skb;

	skb_push(skb, ETH_HLEN);

	if (dst_len < skb->len) {
		printk("uplink_tx_packet_content: packet exceeds uplink packet size\n");
		memset(dst, 0, dst_len);
		return 0;
	}

	skb_copy_from_linear_data(skb, dst, skb->len);

	/* clear unused part of the destination buffer */
	memset(dst + skb->len, 0, dst_len - skb->len);

	return skb->len;
}

static rx_handler_result_t handle_rx(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct net_device *dev = skb->dev;
	struct genode_uplink_tx_packet_context ctx = { .skb = skb };

	{
		bool progress = genode_uplink_tx_packet(dev_genode_uplink(dev),
		                                        uplink_tx_packet_content,
		                                        &ctx);
		if (!progress)
			printk("handle_rx: uplink saturated, dropping packet\n");
	}

	kfree_skb(skb);
	return RX_HANDLER_CONSUMED;
}

/**
 * Create Genode uplink for given net device
 *
 * The uplink is registered at the dev->ifalias pointer.
 */
static void handle_create_uplink(struct net_device *dev)
{
	struct genode_uplink_args args;

	if (dev_genode_uplink(dev))
		return;

	if (!netif_carrier_ok(dev))
		return;

	printk("create uplink for net device %s\n", &dev->name[0]);

	memset(&args, 0, sizeof(args));

	if (dev->addr_len != sizeof(args.mac_address)) {
		printk("error: net device has unexpected addr_len %u\n", dev->addr_len);
		return;
	}

	{
		unsigned i;
		for (i = 0; i < dev->addr_len; i++)
			args.mac_address[i] = dev->dev_addr[i];
	}

	args.label = &dev->name[0];

	dev->ifalias = (struct dev_ifalias *)genode_uplink_create(&args);
}

static void handle_destroy_uplink(struct net_device *dev)
{
	struct genode_uplink *uplink = dev_genode_uplink(dev);

	if (!uplink)
		return;

	if (netif_carrier_ok(dev))
		return;

	genode_uplink_destroy(uplink);

	dev->ifalias = NULL;
}


static genode_uplink_rx_result_t uplink_rx_one_packet(struct genode_uplink_rx_context *ctx,
                                                      char const *ptr, unsigned long len)
{
	struct sk_buff *skb = alloc_skb(len, GFP_KERNEL);

	if (!skb) {
		printk("alloc_skb failed\n");
		return GENODE_UPLINK_RX_RETRY;
	}

	skb_copy_to_linear_data(skb, ptr, len);
	skb_put(skb, len);
	skb->dev = ctx->dev;

	if (dev_queue_xmit(skb) < 0) {
		printk("lx_user: failed to xmit packet\n");
		return GENODE_UPLINK_RX_REJECTED;
	}

	return GENODE_UPLINK_RX_ACCEPTED;
}


static int user_task_function(void *arg)
{
	for (;;) {

		struct net_device *dev;

		for_each_netdev(&init_net, dev) {
			rtnl_lock();
			/* enable link sensing, repeated calls are handled by testing IFF_UP */
			dev_open(dev, 0);

			/* install rx handler once */
			if (!netdev_is_rx_handler_busy(dev))
				netdev_rx_handler_register(dev, handle_rx, NULL);

			/* respond to cable plug/unplug */
			handle_create_uplink(dev);
			handle_destroy_uplink(dev);

			/* transmit packets received from the uplink session */
			if (netif_carrier_ok(dev)) {

				struct genode_uplink_rx_context ctx = { .dev = dev };

				while (genode_uplink_rx(dev_genode_uplink(dev),
				                        uplink_rx_one_packet,
				                        &ctx));
			}

			rtnl_unlock();
		};

		/* block until lx_emul_task_unblock */
		lx_emul_task_schedule(true);
	}
	return 0;
}

void lx_user_init(void)
{
	pid_t pid;

	pid = kernel_thread(user_task_function, NULL, CLONE_FS | CLONE_FILES);

	user_task_struct_ptr = find_task_by_pid_ns(pid, NULL);
}

