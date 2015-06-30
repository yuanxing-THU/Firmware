#include <initializer_list>
#include <string.h>
#include <stdio.h>

#include "activity_lib_constants.h"
#include "allowed_params.hpp"

namespace Activity {

bool allowed_params_inited = false;
AllowedParam ALLOWED_PARAMS[ALLOWED_PARAM_COUNT];

AllowedParam::AllowedParam() {
    name = nullptr;
    display_name = nullptr;
    display_values = nullptr;
}

AllowedParam::AllowedParam(int _id, const char * _name, param_target_device _target_device, const char * _display_name, const char * const * _display_values) {
    id = _id;
    name = _name;

    target_device = _target_device;
    display_name = _display_name;

    display_values = _display_values;
    for (display_value_count = 0; display_values[display_value_count]!=nullptr; display_value_count++);
}

const char * const DISPLAY_VALUES_EMPTY[]         = {nullptr};
const char * const DISPLAY_VALUES_NAV_AFOL_MODE[] = {"Absolute follow", "Follow path", "Cable park", nullptr};
const char * const DISPLAY_VALUES_ON_OFF[]        = {"On", "Off", nullptr};

bool 
init_allowed_params() {

    ALLOWED_PARAMS[0] = AllowedParam(0,    "A_ACTIVITY",       ALL,    "Activity", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[1] = AllowedParam(1,    "A_BT_CONNECT_TO",  DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[2] = AllowedParam(2,    "A_BT_SVC_MODE",    DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[3] = AllowedParam(3,    "PAFOL_BW_DST_LIM", DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[4] = AllowedParam(4,    "PAFOL_VPID_I",     DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[5] = AllowedParam(5,    "PAFOL_VPID_P",     DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[6] = AllowedParam(6,    "PAFOL_VPID_D",     DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[7] = AllowedParam(7,    "PAFOL_OPT_D",      DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[8] = AllowedParam(8,    "NAV_TAKEOFF_ALT",  DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[9] = AllowedParam(9,    "NAV_AFOL_MODE",    DOG,    "", DISPLAY_VALUES_NAV_AFOL_MODE);
    ALLOWED_PARAMS[10] = AllowedParam(10,   "FOL_VEL_FF",       DOG,    "Follow velocity feed foward", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[11] = AllowedParam(11,   "FOL_USE_ALT",      DOG,    "", DISPLAY_VALUES_ON_OFF);
    ALLOWED_PARAMS[12] = AllowedParam(12,   "FOL_RPT_ALT",      DOG,    "", DISPLAY_VALUES_ON_OFF);
    ALLOWED_PARAMS[13] = AllowedParam(13,   "FOL_VEL_FF",       DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[14] = AllowedParam(14,   "MCP_XY_VEL_P",     DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[15] = AllowedParam(15,   "MCP_XY_VEL_I",     DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[16] = AllowedParam(16,   "MCP_XY_VEL_D",     DOG,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[17] = AllowedParam(17,   "MCP_XY_VEL_P",     LEASH,  "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[18] = AllowedParam(18,   "MCP_XY_VEL_I",     LEASH,  "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[19] = AllowedParam(19,   "MCP_XY_VEL_D",     LEASH,  "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[20] = AllowedParam(20,   "PAFOL_TRAJ_RAD",   ALL,    "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[21] = AllowedParam(21,   "MCP_XY_FF",        DOG,    "", DISPLAY_VALUES_EMPTY);

    allowed_params_inited = true;
    return 0;
}

}
// end of namespace Activity

