REQUIRES = arm_v8a

## Enable debugs log in every linux's compilation units
#CC_OPT += -DDEBUG
CC_C_OPT += -Wno-declaration-after-statement

SRC_S   += lx_kit/spec/arm_v8/setjmp.S

SRC_C += generated_dummies.c

vpath %.c   $(PRG_DIR)

DRIVER = imx_sd_card_drv
BOARDS                       := imx8mp_iot_gate mnt_reform2 imx8q_evk
DTS_PATH(imx8mp_iot_gate)    := arch/arm64/boot/dts/compulab/iot-gate-imx8plus.dts
DTS_EXTRACT(imx8mp_iot_gate) := --select usdhc3
DTS_PATH(mnt_reform2)        := arch/arm64/boot/dts/freescale/imx8mq-mnt-reform2.dts
DTS_EXTRACT(mnt_reform2)     := --select usdhc1 --select usdhc2
DTS_PATH(imx8q_evk)          := arch/arm64/boot/dts/freescale/imx8mq-evk.dts
DTS_EXTRACT(imx8q_evk)       := --select usdhc2

include $(REP_DIR)/src/drivers/sd_card/target.inc

