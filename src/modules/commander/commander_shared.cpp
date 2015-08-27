#include <nuttx/config.h>

#include "commander_shared.hpp"

#include <quick_log/quick_log.hpp>

#include "px4_custom_mode.h"
#include "safety_action_helper.hpp"

// Emm, everyone else defines MAV_MODE_FLAG themselves, let's blindly follow the style ...
// you just can't include it from its original location in common/common.h, somewhere in mavlink.
enum MAV_MODE_FLAG {
	MAV_MODE_FLAG_CUSTOM_MODE_ENABLED = 1, /* 0b00000001 Reserved for future use. | */
	MAV_MODE_FLAG_TEST_ENABLED = 2, /* 0b00000010 system has a test mode enabled. This flag is intended for temporary system tests and should not be used for stable implementations. | */
	MAV_MODE_FLAG_AUTO_ENABLED = 4, /* 0b00000100 autonomous mode enabled, system finds its own goal positions. Guided flag can be set or not, depends on the actual implementation. | */
	MAV_MODE_FLAG_GUIDED_ENABLED = 8, /* 0b00001000 guided mode enabled, system flies MISSIONs / mission items. | */
	MAV_MODE_FLAG_STABILIZE_ENABLED = 16, /* 0b00010000 system stabilizes electronically its attitude (and optionally position). It needs however further control inputs to move around. | */
	MAV_MODE_FLAG_HIL_ENABLED = 32, /* 0b00100000 hardware in the loop simulation. All motors / actuators are blocked, but internal software is full operational. | */
	MAV_MODE_FLAG_MANUAL_INPUT_ENABLED = 64, /* 0b01000000 remote control input is enabled. | */
	MAV_MODE_FLAG_SAFETY_ARMED = 128, /* 0b10000000 MAV safety set to armed. Motors are enabled / running / can start. Ready to fly. | */
	MAV_MODE_FLAG_ENUM_END = 129, /*  | */
};

extern Safety_action_helper g_safety_action_helper;

void commander_shared_preprocess_vehicle_command(struct vehicle_command_s * cmd) {
    if ( cmd->command == VEHICLE_CMD_NAV_REMOTE_CMD ) {
        const unsigned param1 = unsigned(cmd->param1 + 0.5f);
        if ( param1 == REMOTE_CMD_LAND_DISARM && !g_safety_action_helper.Allowed_to_land() ) {
            if ( g_safety_action_helper.Allowed_to_rth() ) {
                cmd->command = VEHICLE_CMD_DO_SET_MODE;
                cmd->param1 = uint8_t( /*airdog_status.base_mode |*/ MAV_MODE_FLAG_SAFETY_ARMED | MAV_MODE_FLAG_CUSTOM_MODE_ENABLED);
                cmd->param2 = PX4_CUSTOM_MAIN_MODE_RTL;
                cmd->param3 = 0;
                cmd->param7 = -1.0f; // Mark this as a fake/partial message, without airdog_status_s.base_mode included.
            } else {
                QLOG_literal("[commander] nothing to transform forbidden land command into");
            }
        }
    } else if ( cmd->command == VEHICLE_CMD_DO_SET_MODE ) {
        const unsigned param2 = unsigned(cmd->param2 + 0.5f);
        if ( param2 == PX4_CUSTOM_MAIN_MODE_RTL && !g_safety_action_helper.Allowed_to_rth() ) {
            if ( g_safety_action_helper.Allowed_to_land() ) {
                cmd->command = VEHICLE_CMD_NAV_REMOTE_CMD;
                cmd->param1 = REMOTE_CMD_LAND_DISARM;
                cmd->param2 = 0;
                cmd->param3 = 0;
                cmd->param4 = 0;
                cmd->param5 = 0;
                cmd->param6 = 0;
                cmd->param7 = 0;
            } else {
                QLOG_literal("[commander] nothing to transform forbidden rth command into");
            }
        }
    }
}
