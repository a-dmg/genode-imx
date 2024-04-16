/*
 * \brief  i.MX FEC Linux kernel functions requiering emulation
 * \author Stefan Kalkowski
 * \author Alice Domage
 * \date   2024-04-15
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 * Copyright (C) 2024 gapfruit ag
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>
#include <lx_emul/random.h>


#include <linux/fs.h>

int alloc_chrdev_region(dev_t * dev,unsigned baseminor,unsigned count,const char * name)
{
	static dev_t counter = 0;
	*dev = counter++;
	return 0;
}


struct net_device;

int netdev_register_kobject(struct net_device * ndev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/slab.h>

struct kmem_cache * kmem_cache_create_usercopy(const char * name,
                                               unsigned int size,
                                               unsigned int align,
                                               slab_flags_t flags,
                                               unsigned int useroffset,
                                               unsigned int usersize,
                                               void (* ctor)(void *))
{
    return kmem_cache_create(name, size, align, flags, ctor);
}

#include <linux/clk.h>

struct clk *devm_clk_get(struct device *dev, const char *id)
{
	return clk_get(dev, id);
}


struct clk *devm_clk_get_optional(struct device *dev,const char *id)
{
	return devm_clk_get(dev, id);
}


#include <linux/dma-mapping.h>

void * dmam_alloc_attrs(struct device * dev,size_t size,dma_addr_t * dma_handle,gfp_t gfp,unsigned long attrs)
{
	void *vaddr;

	vaddr = dma_alloc_attrs(dev, size, dma_handle, gfp, attrs);

	if (!vaddr) {
		return NULL;
	}

	return vaddr;
}


#include <linux/gfp.h>
#include <linux/slab.h>

unsigned long get_zeroed_page(gfp_t gfp_mask)
{
	return (unsigned long)__alloc_pages(GFP_KERNEL, 0, 0, NULL)->virtual;
}

#include <linux/version.h>
#include <linux/gfp.h>

void *page_frag_alloc_align(struct page_frag_cache *nc,
                            unsigned int fragsz, gfp_t gfp_mask,
                            unsigned int align_mask)
{
	unsigned int const order = fragsz / PAGE_SIZE;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,12,0)
	struct page *page = __alloc_pages(gfp_mask, order, 0, NULL);
#else
	struct page *page = __alloc_pages(gfp_mask, order, 0);
#endif

	if (!page)
		return NULL;

	/* see page_frag_free */
	if (order > 0)
		printk("%s: alloc might leak memory: fragsz: %u PAGE_SIZE: %lu "
		       "order: %u page: %p addr: %p\n", __func__, fragsz, PAGE_SIZE, order, page, page->virtual);

	return page_address(page);
}


#include <linux/gfp.h>

void page_frag_free(void * addr)
{
	struct page *page = virt_to_page(addr);
	if (!page) {
		printk("BUG %s: page for addr: %p not found\n", __func__, addr);
		lx_emul_backtrace();
	}

	__free_pages(page, 0ul);
}



#include <linux/slab.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/pseudo_fs.h>

struct pseudo_fs_context * init_pseudo(struct fs_context *fc,
                                       unsigned long magic)
{
	struct pseudo_fs_context *pfs_ctx;

	pfs_ctx = kzalloc(sizeof (struct pseudo_fs_context), GFP_KERNEL);
	if (pfs_ctx) {
		pfs_ctx->magic = magic;
		fc->fs_private = pfs_ctx;
	}
	return pfs_ctx;
}

struct inode * new_inode_pseudo(struct super_block * sb)
{
	const struct super_operations *ops = sb->s_op;
	struct inode *inode;

	if (ops->alloc_inode)
		inode = ops->alloc_inode(sb);

	if (!inode)
		return (struct inode*)ERR_PTR(-ENOMEM);

	if (!inode->free_inode)
		inode->free_inode = ops->free_inode;

	return inode;
}



struct vfsmount * kern_mount(struct file_system_type * type)
{
	struct vfsmount *m;

	/*
	 * This sets everything up so that 'new_inode_pseudo()'
	 * called from 'sock_alloc()' properly allocates the inode.
	 */

	m = kzalloc(sizeof (struct vfsmount), 0);
	if (m) {

		struct fs_context fs_ctx;

		if (type->init_fs_context) {
			type->init_fs_context(&fs_ctx);

			m->mnt_sb = kzalloc(sizeof (struct super_block), GFP_KERNEL);
			m->mnt_sb->s_type = type;
			m->mnt_sb->s_op =
				((struct pseudo_fs_context*)fs_ctx.fs_private)->ops;
		} else {
			kfree(m);
			m = (struct vfsmount*)ERR_PTR(-ENOMEM);
		}
	}

	return m;
}

#include <../mm/slab.h>

void * kmem_cache_alloc_lru(struct kmem_cache * cachep,struct list_lru * lru,gfp_t flags)
{
	return kmalloc(cachep->size, flags);
}


#include <linux/slab.h>

int kmem_cache_alloc_bulk(struct kmem_cache * s,gfp_t flags,size_t size,void ** p)
{
	*p = kmalloc(size, flags);
	return size;
}


#include <linux/slab.h>

void kmem_cache_free_bulk(struct kmem_cache * s,size_t size,void ** p)
{
	lx_emul_trace_and_stop(__func__);
}


