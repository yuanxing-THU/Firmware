MODULE_COMMAND = $(notdir $(shell pwd))

SRCS = main.cpp

EXTRACXXFLAGS = -std=c++11 \
		 -Dmain=$(MODULE_COMMAND)_main \
# end of EXTRACXXFLAGS
