#
# Latest board revision is defined in board_AirDogFMU.mk
#

MODULES += drivers/boards/AirLeash/$(CONFIG_BOARD_REVISION)
MODULES += drivers/boards/AirLeash/kbd

ROMFS_EXTRA_FILES = $(PX4_BASE)ROMFS/factory/AirLeash/settings

include $(PX4_BASE)/makefiles/config_Factory_common.mk
