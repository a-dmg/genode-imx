TARGET     := fec_nic_drv

LIBS        = base imx_linux_generated imx_lx_emul jitterentropy
INC_DIR    += $(PRG_DIR)/../..

SRC_C  += dummies.c
SRC_C  += lx_emul.c
SRC_C  += lx_user.c
SRC_CC += main.cc

vpath %.c   $(REP_DIR)/src/drivers/nic/fec
vpath %.cc  $(REP_DIR)/src/drivers/nic/fec

SRC_C  += lx_emul/clocksource.c
SRC_CC += lx_emul/random.cc

SRC_C += lx_emul/shadow/drivers/base/power/runtime.c
SRC_C += lx_emul/shadow/drivers/char/random.c
SRC_C += lx_emul/shadow/drivers/clk/clk.c
SRC_C += lx_emul/shadow/drivers/clk/clk.c
SRC_C += lx_emul/shadow/drivers/clk/clkdev.c
SRC_C += lx_emul/shadow/drivers/clk/clkdev.c
SRC_C += lx_emul/shadow/fs/libfs.c
SRC_C += lx_emul/shadow/kernel/dma/mapping.c
SRC_C += lx_emul/shadow/kernel/dma/mapping.c
SRC_C += lx_emul/shadow/kernel/irq/spurious.c
SRC_C += lx_emul/shadow/kernel/irq/spurious.c
SRC_C += lx_emul/shadow/kernel/rcu/srcutree.c
SRC_C += lx_emul/shadow/kernel/rcu/srcutree.c
SRC_C += lx_emul/shadow/lib/devres.c
SRC_C += lx_emul/shadow/lib/devres.c
SRC_C += lx_emul/shadow/lib/smp_processor_id.c
SRC_C += lx_emul/shadow/lib/smp_processor_id.c
SRC_C += lx_emul/shadow/mm/memblock.c
SRC_C += lx_emul/shadow/mm/memblock.c
SRC_C += lx_emul/shadow/mm/page_alloc.c
SRC_C += lx_emul/shadow/mm/page_alloc.c
SRC_C += lx_emul/shadow/mm/vmalloc.c

SRC_CC  += genode_c_api/uplink.cc
GENODE_C_API_SRC_DIR := $(call select_from_repositories,src/lib/genode_c_api)
vpath % $(dir $(GENODE_C_API_SRC_DIR))

