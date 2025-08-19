
BOARD = imx93_phyboard_nash
BUILD_MODE = dev

UBOOT_DEFAULT_ENV = spec/dev/dev.env

UBOOT_EXTRA_CONFIG := $(REP_DIR)/src/u-boot/$(BOARD)/spec/dev/dev.cfg

include $(call select_from_repositories,src/u-boot/imx93_phyboard_nash/target.inc)
