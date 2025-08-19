
BOARD = imx8mp_phyboard_pollux
BUILD_MODE = sd

UBOOT_DEFAULT_ENV = spec/sd/sd.env

UBOOT_EXTRA_CONFIG := $(REP_DIR)/src/u-boot/$(BOARD)/config/default.cfg \
                      $(REP_DIR)/src/u-boot/$(BOARD)/config/ram.cfg \
                      $(REP_DIR)/src/u-boot/$(BOARD)/config/tpm2.cfg \
                      $(REP_DIR)/src/u-boot/$(BOARD)/config/log.cfg \
                      $(REP_DIR)/src/u-boot/$(BOARD)/spec/sd/version.cfg

include $(call select_from_repositories,src/u-boot/imx8mp_phyboard_pollux/target.inc)
