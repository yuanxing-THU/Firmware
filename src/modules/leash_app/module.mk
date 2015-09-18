MODULE_COMMAND		= $(notdir $(shell pwd))

SRCS			= \
    modes/acquiring_gps.cpp \
    modes/base.cpp \
    modes/calibrate.cpp \
    modes/connect.cpp \
    modes/error.cpp \
    modes/list.cpp \
    modes/logo.cpp \
    modes/main.cpp \
    modes/menu.cpp \
    modes/sensorcheck.cpp \
    modes/service.cpp \
    main.cpp  \
    uorb_functions.cpp  \
    datamanager.cpp \
    displayhelper.cpp \
    button_handler.cpp \

# Required by mavlink_bridge_header
INCLUDE_DIRS	 += $(MAVLINK_SRC)

DEFAULT_VISIBILITY = protected
CXXFLAGS		+= -std=c++11 -Dmain=${MODULE_COMMAND}_main
