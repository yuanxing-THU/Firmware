MODULE_COMMANDS = \
	desktop.SCHED_PRIORITY_DEFAULT.CONFIG_PTHREAD_STACK_DEFAULT.desktop_mobile_main \
	mobile.SCHED_PRIORITY_DEFAULT.CONFIG_PTHREAD_STACK_DEFAULT.desktop_mobile_main \
#end of MODULE_COMMANDS

SRCS = main.cpp
TOLERATE_MISSING_DECLARATION = yes

EXTRACXXFLAGS = -std=c++11 \
		 -Dmain=$(MODULE_COMMAND)_main \
# end of EXTRACXXFLAGS

#
# All error messages are essential for finding conflicting template candidates.
#
SHOW_ALL_ERRORS = yes
