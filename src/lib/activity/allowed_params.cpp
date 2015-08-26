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
const char * const DISPLAY_VALUES_NAV_AFOL_MODE[] = {"Absolute follow", "Follow path", "Cable park", nullptr};
const char * const DISPLAY_VALUES_ON_OFF[]        = {"On", "Off", nullptr};

bool 
init_allowed_params() {

    ALLOWED_PARAMS[0] = AllowedParam(0,     "A_ACTIVITY",       ALL, "", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[1] = AllowedParam(1,     "NAV_AFOL_MODE",    DOG, "", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[2] = AllowedParam(2,     "NAV_TAKEOFF_ALT",  DOG, "m", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[3] = AllowedParam(3,     "A_INIT_POS_U",     DOG, "", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[4] = AllowedParam(4,     "A_INIT_POS_D",     DOG, "m", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[5] = AllowedParam(5,     "V_REACTION",       DOG, "", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[6] = AllowedParam(6,     "FOL_RPT_ALT",      DOG, "m", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[7] = AllowedParam(7,     "SENS_SON_MIN",     DOG, "m", "", DISPLAY_VALUES_NAV_AFOL_MODE);
    ALLOWED_PARAMS[8] = AllowedParam(8,     "PAFOL_OPT_D",      DOG, "m", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[9] = AllowedParam(9,     "AIRD_TRAJ_RAD",    ALL, "m", "", DISPLAY_VALUES_ON_OFF);
    ALLOWED_PARAMS[10] = AllowedParam(10,   "FOL_LPF_XY",       DOG, "", "", DISPLAY_VALUES_ON_OFF);
    ALLOWED_PARAMS[11] = AllowedParam(11,   "FOL_VEL_FF_XY",    DOG, "", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[12] = AllowedParam(12,   "BAT_WARN_LVL",     DOG, "%", "", DISPLAY_VALUES_EMPTY);
    ALLOWED_PARAMS[13] = AllowedParam(13,   "BAT_CRIT_LVL",     DOG, "%", "", DISPLAY_VALUES_EMPTY);

    allowed_params_inited = true;
    return true;

}
}
// end of namespace Activity

