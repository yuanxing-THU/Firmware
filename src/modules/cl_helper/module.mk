#
# Some commands to do stuff from commandline for testing purposes. 
# For example simultating drone button functionality.
#

MODULE_COMMAND		= clh

SRCS			= main.cpp

EXTRACXXFLAGS += -std=c++11 \
			-Dmain=${MODULE_COMMAND}_main \
			#-flto \
# end of EXTRACXXFLAGS

MODULE_STACKSIZE = 5200

MAXOPTIMIZATION = -Os
