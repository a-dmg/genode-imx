
BOARD = imx93_phyboard_nash
BUILD_MODE = dev

UBOOT_DEFAULT_ENV = spec/dev/dev.env

UBOOT_EXTRA_CONFIG := $(REP_DIR)/src/u-boot/$(BOARD)/config/default.cfg
# UBOOT_EXTRA_CONFIG := $(REP_DIR)/src/u-boot/$(BOARD)/config/default.cfg \
#                       $(REP_DIR)/src/u-boot/$(BOARD)/spec/$(BUILD_MODE)/version.cfg \
#                       $(REP_DIR)/src/u-boot/$(BOARD)/config/log.cfg \
#                       $(REP_DIR)/src/u-boot/$(BOARD)/config/tpm2.cfg \
#                       $(REP_DIR)/src/u-boot/$(BOARD)/spec/dev/dev.cfg

include $(call select_from_repositories,src/u-boot/imx93_phyboard_nash/target.inc)
