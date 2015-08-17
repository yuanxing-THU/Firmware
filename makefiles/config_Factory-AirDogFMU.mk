#
# Latest board revision is defined in board_AirDogFMU.mk
#

MODULES += drivers/boards/AirDogFMU/$(CONFIG_BOARD_REVISION)
MODULES += modules/i2c_exchange

include $(PX4_BASE)/makefiles/config_Factory_common.mk
