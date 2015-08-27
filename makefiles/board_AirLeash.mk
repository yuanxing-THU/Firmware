#
# Board-specific definitions for AirLeash
#

#
# Configure the toolchain
#
CONFIG_ARCH			 = CORTEXM4F
CONFIG_BOARD			 = AIRLEASH

#
# Latest board electronics revision
#
# It should be overritten by older board configs.
# Most actual board config should not define it.
#
CONFIG_BOARD_REVISION ?= 006

include $(PX4_MK_DIR)/toolchain_gnu-arm-eabi.mk
