#include <nuttx/config.h>

#include "commander_shared.hpp"

#include <mavlink/mavlink_bridge_header.h>
#include <quick_log/quick_log.hpp>

#include "px4_custom_mode.h"
#include "safety_action_helper.hpp"

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
