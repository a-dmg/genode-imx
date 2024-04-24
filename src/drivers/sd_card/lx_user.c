/**
 * \brief  Genode <=> Linux kernel interaction - Linux side
 * \author Stefan Kalkowski
 * \author Alice Domage
 * \date   2024-04-17
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 * Copyright (C) 2024 gapfruit ag
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <../block/blk.h>
#include <../block/blk-mq.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/part_stat.h>
#include <linux/backing-dev.h>
#include <uapi/linux/hdreg.h>

#include <genode_c_api/block.h>
#include <lx_emul/debug.h>
#include <lx_user/init.h>
#include <lx_user/io.h>

static struct task_struct         *lx_user_task = NULL;
static  __cacheline_aligned_in_smp DEFINE_SPINLOCK(bdev_lock);


static bool bd_may_claim(struct block_device *bdev,
                         struct block_device *whole,
                         void                *holder)
{
	if (bdev->bd_holder == holder)
		return true;	 /* already a holder */
	else if (bdev->bd_holder != NULL)
		return false; 	 /* held by someone else */
	else if (whole == bdev)
		return true;  	 /* is a whole device which isn't held */

	else if (whole->bd_holder == bd_may_claim)
		return true; 	 /* is a partition of a device that is being partitioned */
	else if (whole->bd_holder != NULL)
		return false;	 /* is a partition of a held device */
	else
		return true;	 /* is a partition of an un-held device */
}


int bd_prepare_to_claim(struct block_device * bdev,void * holder)
{
	struct block_device *whole = bdev_whole(bdev);

	if (WARN_ON_ONCE(!holder))
		return -EINVAL;
retry:
	spin_lock(&bdev_lock);
	/* if someone else claimed, fail */
	if (!bd_may_claim(bdev, whole, holder)) {
		spin_unlock(&bdev_lock);
		return -EBUSY;
	}

	/* if claiming is already in progress, wait for it to finish */
	if (whole->bd_claiming) {
		wait_queue_head_t *wq = bit_waitqueue(&whole->bd_claiming, 0);
		DEFINE_WAIT(wait);

		prepare_to_wait(wq, &wait, TASK_UNINTERRUPTIBLE);
		spin_unlock(&bdev_lock);
		schedule();
		finish_wait(wq, &wait);
		goto retry;
	}

	/* yay, all mine */
	whole->bd_claiming = holder;
	spin_unlock(&bdev_lock);
	return 0;
}


static int __init genhd_device_init(void)
{
printk("%s() : %d\n", __FUNCTION__, __LINE__);
	return blk_dev_init();
}
subsys_initcall(genhd_device_init);


struct block_device *bdev_alloc(struct gendisk *disk, u8 partno)
{
	struct block_device * bdev  = kzalloc(sizeof(struct block_device), GFP_KERNEL);
	struct inode        * inode = kzalloc(sizeof(struct inode), GFP_KERNEL);

	inode->i_mode = S_IFBLK;
	inode->i_rdev = 0;

	spin_lock(&bdev_lock);
	mutex_init(&bdev->bd_fsfreeze_mutex);
	spin_lock_init(&bdev->bd_size_lock);
	bdev->bd_disk = disk;
	bdev->bd_partno = partno;
	bdev->bd_inode = inode;
	bdev->bd_queue = disk->queue;

	bdev->bd_stats = alloc_percpu(struct disk_stats);
	spin_unlock(&bdev_lock);
	return bdev;
}


enum { MAX_BDEV = 256 };
static struct block_device * bdevs[MAX_BDEV];

void bdev_add(struct block_device * bdev, dev_t dev)
{
	unsigned idx = MAJOR(dev);
	bdev->bd_dev = dev;
	if (idx < MAX_BDEV) {
		bdevs[idx] = bdev;
		return;
	}

	printk("Error: bdev_add invalid major=%u minor=%u\n",
	       MAJOR(dev), MINOR(dev));
	lx_emul_trace_and_stop(__func__);
}


struct block_device * blkdev_get_by_dev(dev_t dev,fmode_t mode,void * holder)
{
	unsigned idx = MAJOR(dev);
	if (idx < MAX_BDEV)
		return bdevs[idx];

	printk("Error: blkdev_get_by_dev invalid major=%u minor=%u\n",
	       MAJOR(dev), MINOR(dev));
	return NULL;
}

enum { MAX_GEN_DISKS = 4 };

struct gendisk * gendisks[MAX_GEN_DISKS];

int blk_register_queue(struct gendisk * disk)
{
	unsigned idx;
	for (idx = 0; idx < MAX_GEN_DISKS; idx++)
		if (!gendisks[idx])
			break;

	if (idx >= MAX_GEN_DISKS)
		return -1;

	genode_block_announce_device(disk->disk_name, get_capacity(disk),
	                             get_disk_ro(disk) ? 0 : 1);
	gendisks[idx] = disk;
	return 0;
}


void blk_unregister_queue(struct gendisk * disk)
{
	unsigned idx;
	for (idx = 0; idx < MAX_GEN_DISKS; idx++)
		if (gendisks[idx] == disk)
			gendisks[idx] = NULL;

	genode_block_discontinue_device(disk->disk_name);
}



static void bio_end_io(struct bio *bio)
{
	struct genode_block_request * const req =
		(struct genode_block_request*) bio->bi_private;
	struct genode_block_session * const session =
		genode_block_session_by_name(bio->bi_bdev->bd_disk->disk_name);

	if (session) {
		genode_block_ack_request(session, req, true);
		lx_user_handle_io();
	} else
		printk("Error: could not find session or gendisk for bio %p\n", bio);

	bio_put(bio);
}


static inline int block_sync(struct block_device * const bdev)
{
	if (blkdev_issue_flush(bdev)) {
		printk("blkdev_issue_flush failed!\n");
		return 0;
	}
	return 1;
}


static inline void block_request(struct block_device         * const bdev,
                                 struct genode_block_request * const request,
                                 bool                          write)
{
	struct bio  * bio  = bio_alloc(bdev, 1, write ? REQ_OP_WRITE : REQ_OP_READ, GFP_KERNEL);
	struct page * page = virt_to_page(request->addr);

	bio_set_dev(bio, bdev);

	bio->bi_iter.bi_sector = request->blk_nr;
	bio->bi_end_io         = bio_end_io;
	bio->bi_opf            = write ? REQ_OP_WRITE : REQ_OP_READ;
	bio->bi_private        = request;

	bio_add_page(bio, page, request->blk_cnt * 512,
	             (unsigned long)request->addr & (PAGE_SIZE-1));
	submit_bio(bio);
}


static inline void
block_handle_session(struct genode_block_session * const session,
                     struct gendisk              * const disk)
{
	if (!session)
		return;

	for (;;) {
		struct genode_block_request * const req =
			genode_block_request_by_session(session);
		struct block_device * const bdev = disk->part0;

		if (!req)
			return;

		switch (req->op) {
		case GENODE_BLOCK_READ:
			block_request(bdev, req, false);
			break;
		case GENODE_BLOCK_WRITE:
			block_request(bdev, req, true);
			break;
		case GENODE_BLOCK_SYNC:
			genode_block_ack_request(session, req, block_sync(bdev));
		default: ;
		};
	}
}


static int block_poll_sessions(void * data)
{
	for (;;) {
		unsigned idx;

		for (idx = 0; idx < MAX_GEN_DISKS; idx++) {

			struct gendisk * const disk = gendisks[idx];

			if (!disk)
				continue;

			block_handle_session(genode_block_session_by_name(disk->disk_name),
			                     disk);
		}

		lx_emul_task_schedule(true);
	}

	return 0;
}


void lx_user_handle_io(void)
{
	if (lx_user_task)
		lx_emul_task_unblock(lx_user_task);
}


void lx_user_init(void)
{
	int pid = kernel_thread(block_poll_sessions, NULL, CLONE_FS | CLONE_FILES);
	lx_user_task = find_task_by_pid_ns(pid, NULL);
}

