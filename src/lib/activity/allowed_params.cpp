#include <initializer_list>
#include <string.h>
#include <stdio.h>

#include "activity_lib_constants.h"
#include "allowed_params.hpp"

namespace Activity {

bool volatile allowed_params_inited = false;
AllowedParam ALLOWED_PARAMS[ALLOWED_PARAM_COUNT];

AllowedParam::AllowedParam() {
    name = nullptr;
    display_name = nullptr;
    display_values = nullptr;
}

AllowedParam::AllowedParam(int _id, const char * _name, param_target_device _target_device, const char * _units, const char * _display_name, const char * const * _display_values) {

    id = _id;
    name = _name;

    target_device = _target_device;

    units = _units;
    display_name = _display_name;

    display_values = _display_values;
    for (display_value_count = 0; display_values[display_value_count]!=nullptr; display_value_count++);
}

const char * const DISPLAY_VALUES_EMPTY[]         = {nullptr};
const char * const DISPLAY_VALUES_NAV_AFOL_MODE[] = {"Fixed", "Path", "Line", "Adaptive", "Circle", nullptr};
const char * const DISPLAY_VALUES_ON_OFF[]        = {"Off", "On", nullptr};
const char * const DISPLAY_VALUES_YES_NO[]        = {"No", "Yes", nullptr};
const char * const DISPLAY_VALUES_REACTION[]      = {"Easy", "Medium", "Aggresive", nullptr};
const char * const DISPLAY_VALUES_RTH_SPOT[]      = {"ERROR", "Home", "Spot", nullptr};

bool 
init_allowed_params() {

    ALLOWED_PARAMS[0] = AllowedParam(0,     "A_ACTIVITY",       ALL, "", "Activity", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[1] = AllowedParam(1,     "NAV_AFOL_MODE",    DOG, "", "Flight mode", DISPLAY_VALUES_NAV_AFOL_MODE);
    ALLOWED_PARAMS[2] = AllowedParam(2,     "A_BSC_SAF_ACT",    DOG, "", "Landing mode", DISPLAY_VALUES_RTH_SPOT);
    ALLOWED_PARAMS[3] = AllowedParam(3,     "RTL_RET_ALT",      DOG, "m", "Return altitude", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[4] = AllowedParam(4,     "NAV_TAKEOFF_ALT",  DOG, "m", "Take-off alt", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[5] = AllowedParam(5,     "A_INIT_POS_U",     DOG, "", "Use initial dst", DISPLAY_VALUES_YES_NO);
    ALLOWED_PARAMS[6] = AllowedParam(6,     "A_INIT_POS_D",     DOG, "m", "Initial dst", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[7] = AllowedParam(7,     "V_REACTION",       DOG, "", "Reaction", DISPLAY_VALUES_REACTION);
    ALLOWED_PARAMS[8] = AllowedParam(8,     "FOL_RPT_ALT",      DOG, "m", "Follow alt", DISPLAY_VALUES_YES_NO);
    ALLOWED_PARAMS[9] = AllowedParam(9,     "SENS_SON_MIN",     DOG, "m", "Lidar alt", DISPLAY_VALUES_NAV_AFOL_MODE);
    ALLOWED_PARAMS[10] = AllowedParam(10,     "PAFOL_OPT_D",      DOG, "m", "Follow Path dst", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[11] = AllowedParam(11,     "AIRD_TRAJ_RAD",    ALL, "m", "Follow Path rad", DISPLAY_VALUES_ON_OFF);
    ALLOWED_PARAMS[12] = AllowedParam(12,   "FOL_LPF_XY",       DOG, "", "Follow LPF xy", DISPLAY_VALUES_ON_OFF);
    ALLOWED_PARAMS[13] = AllowedParam(13,   "FOL_VEL_FF_XY",    DOG, "", "Follow FF xy", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[14] = AllowedParam(14,   "BAT_WARN_LVL",     DOG, "%", "Battery WARN lvl", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[15] = AllowedParam(15,   "BAT_CRIT_LVL",     DOG, "%", "Battery CRIT lvl", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[16] = AllowedParam(16,   "A_SAH_NO_SPOT",    DOG, "", "Enforce RTH", DISPLAY_VALUES_ON_OFF);
    ALLOWED_PARAMS[17] = AllowedParam(17,   "OFF_FR_ADD_ANG",    DOG, "", "Relative angle", DISPLAY_VALUES_ON_OFF);
    

    allowed_params_inited = true;
    return true;
}
}
// end of namespace Activity

