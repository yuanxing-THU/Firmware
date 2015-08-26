#
# Latest board revision is defined in board_AirDogFMU.mk
#

MODULES += drivers/boards/AirDogFMU/$(CONFIG_BOARD_REVISION)

include $(PX4_BASE)/makefiles/config_Factory_common.mk
