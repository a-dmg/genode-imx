TARGET              = imx8mp_iot_gate
REQUIRES            = arm_v8a
CUSTOM_TARGET_DEPS  = flash.bin
UBOOT_CONTRIB_DIR  := $(call select_from_ports,imx8mp_iot_gate_uboot)/uboot
META_BSP_DIR       := $(call select_from_ports,imx8mp_iot_gate_uboot)/meta_bsp_imx8mp
ATF_CONTRIB_DIR    := $(call select_from_ports,imx8mp_iot_gate_uboot)/atf
MKIMG_CONTRIB_DIR  := $(call select_from_ports,imx8mp_iot_gate_uboot)/mkimage
FWI_CONTRIB_DIR    := $(call select_from_ports,imx8mp_iot_gate_uboot)/firmware

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
## u-boot
##
UBOOT_SPL_BIN = $(PWD)/$(PRG_REL_DIR)/uboot/spl/u-boot-spl.bin 
$(UBOOT_SPL_BIN):
	# Configure u-boot
	$(VERBOSE)mkdir -p $(PWD)/$(PRG_REL_DIR)/uboot
	# Generate config
	$(VERBOSE)$(MAKE) -C $(UBOOT_CONTRIB_DIR) O=$(PWD)/$(PRG_REL_DIR)/uboot \
		$(CROSS_COMPILE_OPT) \
		MACHINE=iot-gate-imx8plus \
		iot-gate-imx8plus_defconfig
	$(VERBOSE)$(UBOOT_CONTRIB_DIR)/scripts/kconfig/merge_config.sh -m -O $(PWD)/$(PRG_REL_DIR)/uboot $(PWD)/$(PRG_REL_DIR)/uboot/.config \
		$(META_BSP_DIR)/recipes-bsp/u-boot/compulab/2021.04/imx8mp/common.cfg \
		$(META_BSP_DIR)/recipes-bsp/u-boot/compulab/2021.04/imx8mp/d1d8.cfg \
		$(META_BSP_DIR)/recipes-bsp/u-boot/compulab/2021.04/imx8mp/d2d4.cfg \
		$(REP_DIR)/src/u-boot/imx8mp_iot_gate/default_env_file.cfg \
		$(REP_DIR)/src/u-boot/imx8mp_iot_gate/enable_tpm.cfg \
		$(META_BSP_DIR)/recipes-bsp/u-boot/compulab/2021.04/imx8mp/iot-gate-imx8plus_defconfig
	$(VERBOSE)$(MAKE) -C $(UBOOT_CONTRIB_DIR) O=$(PWD)/$(PRG_REL_DIR)/uboot \
		$(CROSS_COMPILE_OPT) \
		MACHINE=iot-gate-imx8plus \
		oldconfig
	$(VERBOSE)cp $(REP_DIR)/src/u-boot/imx8mp_iot_gate/iot_gate-develop.env $(PWD)/$(PRG_REL_DIR)/uboot/iot_gate.env
	# install lpddr trained set
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/ddr/synopsys/lpddr4_pmu_train_1d_dmem_202006.bin $(PWD)/$(PRG_REL_DIR)/uboot/lpddr4_pmu_train_1d_dmem.bin
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/ddr/synopsys/lpddr4_pmu_train_1d_imem_202006.bin $(PWD)/$(PRG_REL_DIR)/uboot/lpddr4_pmu_train_1d_imem.bin
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/ddr/synopsys/lpddr4_pmu_train_2d_dmem_202006.bin $(PWD)/$(PRG_REL_DIR)/uboot/lpddr4_pmu_train_2d_dmem.bin
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/ddr/synopsys/lpddr4_pmu_train_2d_imem_202006.bin $(PWD)/$(PRG_REL_DIR)/uboot/lpddr4_pmu_train_2d_imem.bin
	# Adjust tag to the git tag of u-boot later
	$(VERBOSE)$(MAKE) -C $(UBOOT_CONTRIB_DIR) O=$(PWD)/$(PRG_REL_DIR)/uboot \
		$(CROSS_COMPILE_OPT) \
		all


##
## Arm Trusted Firmware
##
ATF_BIN = $(PWD)/$(PRG_REL_DIR)/atf/build/imx8mp/release/bl31.bin
$(ATF_BIN):
	# Build arm trusted firmware
	$(VERBOSE)cp -r $(ATF_CONTRIB_DIR) $(PWD)/$(PRG_REL_DIR)/atf
	$(VERBOSE)$(MAKE) -C $(PWD)/$(PRG_REL_DIR)/atf \
		$(CROSS_COMPILE_OPT) \
		PLAT=imx8mp \
		bl31


##
## MK image and flash.bin generation
##
MKIMAGE_BIN = $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/flash.bin
$(MKIMAGE_BIN): $(ATF_BIN) $(UBOOT_SPL_BIN)
	# Copy mkimage project to build dir
	$(VERBOSE)cp -r $(MKIMG_CONTRIB_DIR) $(PWD)/$(PRG_REL_DIR)/mkimage
	# Deploy the arm trusted firmware (bl1 & bl2)
	$(VERBOSE)ln -sf $(ATF_BIN) $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/bl31.bin
	# Deploy u-boot together with the SPL firmware (bl31)
	$(VERBOSE)ln -sf $(UBOOT_SPL_BIN) $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/u-boot-spl.bin
	# Deploy DTBs
	$(VERBOSE)ln -sf $(PWD)/$(PRG_REL_DIR)/uboot/arch/arm/dts/iot-gate-imx8plus.dtb $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/
	$(VERBOSE)ln -sf $(PWD)/$(PRG_REL_DIR)/uboot/u-boot-nodtb.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/
	# Deploy mkimage uboot's script
	$(VERBOSE)ln -sf $(PWD)/$(PRG_REL_DIR)/uboot/tools/mkimage $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/mkimage_uboot
	# Deploy training ddr firmwares
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/ddr/synopsys/lpddr4_pmu_train_1d_dmem_202006.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/ddr/synopsys/lpddr4_pmu_train_1d_imem_202006.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/ddr/synopsys/lpddr4_pmu_train_2d_dmem_202006.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/ddr/synopsys/lpddr4_pmu_train_2d_imem_202006.bin $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/hdmi/cadence/signed_hdmi_imx8m.bin               $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/
	$(VERBOSE)ln -sf $(FWI_CONTRIB_DIR)/firmware-imx-8.16/firmware/hdmi/cadence/signed_dp_imx8m.bin                 $(PWD)/$(PRG_REL_DIR)/mkimage/iMX8M/
	$(VERBOSE)echo "#define MKIMAGE_COMMIT 0x1" > $(PWD)/$(PRG_REL_DIR)/mkimage/src/build_info.h
	# Build mkimage and generate flash.bin
	$(MAKE) -C $(PWD)/$(PRG_REL_DIR)/mkimage -j $(NPROCS) CROSS_COMPILE=$(CROSS_DEV_PREFIX) \
		ARCH=arm \
		SOC=iMX8MP \
		dtbs=iot-gate-imx8plus.dtb \
		flash_evk


##
## Deploy flash.bin
##
$(CUSTOM_TARGET_DEPS): $(MKIMAGE_BIN)
	# Deploy flash.bin
	$(VERBOSE)cp $(MKIMAGE_BIN) $@

