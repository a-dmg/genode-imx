REQUIRES            = arm_v8a
CUSTOM_TARGET_DEPS  = imx-boot
UBOOT_CONTRIB_DIR   := $(call select_from_ports,imx93_phyboard_nash_uboot)/uboot
ATF_CONTRIB_DIR     := $(call select_from_ports,imx93_phyboard_nash_uboot)/atf
FWI_CONTRIB_DIR     := $(call select_from_ports,imx93_phyboard_nash_uboot)/firmware/firmware-imx-8.26-d4c33ab
FWI_ELE_CONTRIB_DIR := $(call select_from_ports,imx93_phyboard_nash_uboot)/firmware-ele/firmware-ele-imx-1.3.0-17945fc
MKIMG_CONTRIB_DIR   := $(call select_from_ports,imx93_phyboard_nash_uboot)/mkimage

# do not confuse third-party sub-makes
unexport BOARD
unexport MAKEFLAGS
unexport .SHELLFLAGS


##
## Cross compile options
##

NPROCS := $(shell grep -c ^processor /proc/cpuinfo)

CROSS_CFLAGS = --warn-no-array-bounds \
               --warn-no-implicit-function-declaration \
               --warn-no-return-type \
               --warn-no-stringop-truncation \
               --warn-no-stringop-overread \
               --warn-no-unused-variable

CROSS_LDFLAGS = --no-warn-rwx-segments

CROSS_COMPILE_OPT = -j $(NPROCS) CROSS_COMPILE=$(CROSS_DEV_PREFIX) KCFLAGS+="$(CROSS_CFLAGS)" CFLAGS+="$(CROSS_CFLAGS)" LDFLAGS="$(CROSS_LDFLAGS)"


##
## Determine if compile_commands.json is needed
##

ifeq ($(COMPILE_COMMANDS),yes)
    UBOOT_BEAR = bear --output $(UBOOT_CONTRIB_DIR)/compile_commands.json --
else
    UBOOT_BEAR = 
endif


##
## Board Configuration
##

include $(REP_DIR)/src/u-boot/imx93_phyboard_nash/epoch.env

IMX_BOOT_UART_BASE := "" # only i.IMX8 M has support to override UART base address, provide dummy value
PLATFORM           := imx93

##
## Arm Trusted Firmware
##
ATF_BIN = $(PWD)/$(PRG_REL_DIR)/atf/build/imx93/release/bl31.bin
$(ATF_BIN):
	# Configure atf build directory
	$(VERBOSE)cp -r $(ATF_CONTRIB_DIR) $(PWD)/$(PRG_REL_DIR)/atf
	# Build atf
	$(VERBOSE)$(MAKE) -C $(PWD)/$(PRG_REL_DIR)/atf \
		$(CROSS_COMPILE_OPT) \
		IMX_BOOT_UART_BASE=$(IMX_BOOT_UART_BASE) \
		PLAT=$(PLATFORM) \
		DEBUG=0 \
		bl31
#	$(VERBOSE)$(MAKE) -C $(PWD)/$(PRG_REL_DIR)/atf \
#		$(CROSS_COMPILE_OPT) \
#		IMX_BOOT_UART_BASE=$(IMX_BOOT_UART_BASE) \
#		PLAT=$(PLATFORM) \
#		BUILD_BASE=build-optee \
#		DEBUG=0 \
#		clean
#	$(VERBOSE)$(MAKE) -C $(PWD)/$(PRG_REL_DIR)/atf \
#		$(CROSS_COMPILE_OPT) \
#		IMX_BOOT_UART_BASE=$(IMX_BOOT_UART_BASE) \
#		PLAT=$(PLATFORM) \
#		BUILD_BASE=build-optee \
#		SPD=opteed \
#		DEBUG=0 \
#		bl31

##
## u-boot
##
UBOOT = $(PWD)/$(PRG_REL_DIR)/uboot/u-boot.bin
UBOOT_SPL = $(PWD)/$(PRG_REL_DIR)/uboot/spl/u-boot-spl.bin
$(UBOOT): export SOURCE_DATE_EPOCH = $(EPOCH)
$(UBOOT): $(ATF_BIN)
	# Configure u-boot build directory
	$(VERBOSE)mkdir -p $(PWD)/$(PRG_REL_DIR)/uboot
	$(VERBOSE)ln -sf $(REP_DIR)/src/u-boot/$(BOARD)/$(UBOOT_DEFAULT_ENV) $(PWD)/$(PRG_REL_DIR)/uboot/default.env
	# Generate config
	$(VERBOSE)$(MAKE) -C $(UBOOT_CONTRIB_DIR) O=$(PWD)/$(PRG_REL_DIR)/uboot \
		$(CROSS_COMPILE_OPT) \
		imx93-phycore_defconfig
	$(VERBOSE)$(UBOOT_CONTRIB_DIR)/scripts/kconfig/merge_config.sh -m -O $(PWD)/$(PRG_REL_DIR)/uboot $(PWD)/$(PRG_REL_DIR)/uboot/.config \
		$(UBOOT_EXTRA_CONFIG)
	$(VERBOSE)$(MAKE) -C $(UBOOT_CONTRIB_DIR) O=$(PWD)/$(PRG_REL_DIR)/uboot \
		$(CROSS_COMPILE_OPT) \
		oldconfig
	# Build u-boot
	$(VERBOSE)$(MAKE) -C $(UBOOT_CONTRIB_DIR) O=$(PWD)/$(PRG_REL_DIR)/uboot \
		$(CROSS_COMPILE_OPT)

##
## MK image and flash.bin generation
##
MKIMAGE_BIN = $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/flash.bin
$(MKIMAGE_BIN): $(ATF_BIN) $(UBOOT)
	# Copy mkimage project to build dir
	$(VERBOSE)cp -r $(MKIMG_CONTRIB_DIR) $(PWD)/$(PRG_REL_DIR)/mkimage
	# Deploy arm trusted firmware
	$(VERBOSE)ln -sf $(ATF_BIN) $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/bl31.bin
	# Deploy u-boot
	$(VERBOSE)ln -sf $(UBOOT) $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/u-boot.bin
	$(VERBOSE)ln -sf $(UBOOT_SPL) $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/u-boot-spl.bin
	# Deploy DTBs
	$(VERBOSE)ln -sf $(PWD)/$(PRG_REL_DIR)/uboot/arch/arm/dts/imx93-phyboard-nash.dtb $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93
	# Deploy mkimage uboot's script
	$(VERBOSE)ln -sf $(PWD)/$(PRG_REL_DIR)/uboot/tools/mkimage $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/mkimage_uboot
	# Deploy ahab container
	$(VERBOSE)ln -sf $(FWI_ELE_CONTRIB_DIR)/mx93a1-ahab-container.img $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/
	# Deploy training ddr firmwares
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware/ddr/synopsys/lpddr4_dmem_1d_v202201.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware/ddr/synopsys/lpddr4_imem_1d_v202201.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware/ddr/synopsys/lpddr4_dmem_2d_v202201.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware/ddr/synopsys/lpddr4_imem_2d_v202201.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX93/
	$(VERBOSE)echo "#define MKIMAGE_COMMIT 0x1" > $(PWD)/$(PRG_REL_DIR)/mkimage/src/build_info.h
	# Build mkimage and generate flash.bin
	$(VERBOSE)ln -sf $(PWD)/$(PRG_REL_DIR)/mkimage_imx8 $(PWD)/$(PRG_REL_DIR)/mkimage/
	$(MAKE) -C $(PWD)/$(PRG_REL_DIR)/mkimage  \
		SOC=iMX93 \
		REV=A1 \
		flash_singleboot

##
## Deploy imx-boot
##
$(CUSTOM_TARGET_DEPS): $(MKIMAGE_BIN)
	$(VERBOSE) ln -sf $(MKIMAGE_BIN) imx-boot

