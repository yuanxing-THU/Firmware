#
# Latest board revision is defined in board_AirDogFMU.mk
#

MODULES += drivers/boards/AirDogFMU/$(CONFIG_BOARD_REVISION)
MODULES += drivers/px4io
MODULES += modules/i2c_exchange

#
# Mobile
#
MODULES += lib/activity
MODULES += lib/airdog/hwinfo
MODULES += lib/stm32f4
MODULES += modules/mobile

ROMFS_EXTRA_FILES = $(PX4_BASE)Images/px4io-v2_default.bin

include $(PX4_BASE)/makefiles/config_Factory_common.mk
