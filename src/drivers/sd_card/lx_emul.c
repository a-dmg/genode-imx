/*
 * \brief  Driver-specific emulation of Linux Kernel functions
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

#include <lx_emul.h>
#include <lx_emul/random.h>

#include <linux/fs.h>

int alloc_chrdev_region(dev_t * dev,unsigned baseminor,unsigned count,const char * name)
{
	static dev_t counter = 0;
	*dev = counter++;
	return 0;
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


#include <linux/mm.h>
#include <../mm/slab.h>

void * kmem_cache_alloc_lru(struct kmem_cache * cachep,struct list_lru * lru,gfp_t flags)
{
	return kmalloc(cachep->size, flags);
}


int cache_line_size(void)
{
	return 128;
}

