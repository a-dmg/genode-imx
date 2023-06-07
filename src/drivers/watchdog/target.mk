TARGET    = imx_watchdog_drv

REQUIRES  = arm_v8

LIBS += base

INC_DIR += $(PRG_DIR)
INC_DIR += $(REP_DIR)/src/drivers/watchdog

SRC_CC += main.cc

vpath %.cc $(REP_DIR)/src/drivers/watchdog
vpath %.cc $(REP_DIR)
