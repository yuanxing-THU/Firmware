#
# Latest board revision is defined in board_AirDogFMU.mk
#

MODULES += drivers/boards/AirLeash/$(CONFIG_BOARD_REVISION)
MODULES += drivers/boards/AirLeash/kbd

include $(PX4_BASE)/makefiles/config_Factory_common.mk
