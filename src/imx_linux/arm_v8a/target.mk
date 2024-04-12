TARGET   := imx_linux
REQUIRES := arm_64

CUSTOM_TARGET_DEPS := kernel_build.phony

LX_DIR := $(call select_from_ports,linux-imx)/linux-imx
PWD    := $(shell pwd)

## To enable debug log, kernel wide, add -> EXTRA_CFLAGS=-DDEBUG
LX_MK_ARGS = ARCH=arm64 CROSS_COMPILE=$(CROSS_DEV_PREFIX)

#
# Linux kernel configuration
#
# define 'LX_ENABLE' and 'LX_DISABLE'
include $(REP_DIR)/src/imx_linux/arm_v8a/target.inc

# filter for make output of kernel build system
BUILD_OUTPUT_FILTER = 2>&1 | sed "s/^/      [Linux]  /"

# do not confuse third-party sub-makes
unexport .SHELLFLAGS

kernel_config.tag:
	$(MSG_CONFIG)Linux
	$(VERBOSE)$(MAKE) -C $(LX_DIR) O=$(PWD) $(LX_MK_ARGS) tinyconfig $(BUILD_OUTPUT_FILTER)
	$(VERBOSE)$(LX_DIR)/scripts/config $(addprefix --enable ,$(LX_ENABLE))
	$(VERBOSE)$(LX_DIR)/scripts/config $(addprefix --disable ,$(LX_DISABLE))
	# Log verbosity can be adjusted here, see include/linux/printk.h
	$(VERBOSE)$(LX_DIR)/scripts/config --set-val MESSAGE_LOGLEVEL_DEFAULT 7
	$(VERBOSE)$(LX_DIR)/scripts/config --set-val CONSOLE_LOGLEVEL_DEFAULT 10 
	$(VERBOSE)$(MAKE) $(LX_MK_ARGS) olddefconfig $(BUILD_OUTPUT_FILTER)
	$(VERBOSE)touch $@

# update Linux kernel config on makefile changes
kernel_config.tag: $(MAKEFILE_LIST)

kernel_build.phony: kernel_config.tag
	$(MSG_BUILD)Linux
	$(VERBOSE)$(MAKE) $(LX_MK_ARGS) dtbs Image Image.gz $(BUILD_OUTPUT_FILTER)

