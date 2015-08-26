#pragma once

#include <stdio.h>
#include <initializer_list>
#include "activity_lib_constants.h"

namespace Activity {

typedef enum {
    DOG = 0,
    LEASH,
    ALL
} param_target_device;


struct AllowedParam {

    uint16_t id;

    const char * name;
    param_target_device target_device;

    const char * display_name;

    uint8_t display_value_count;
    const char * const * display_values;

    const char * units;

    AllowedParam();
    AllowedParam(int _id, const char * _name, param_target_device _target_device, const char * _units, const char * _display_name, const char * const * _display_values); 

};

// List of params in combination with target device for param.
// Only those param - device combinations are allowed to be used in activity params.
//
// Order here is important, never add parameters in the middle of this list.
// If this list is updated make sure there are enough space for all values in *** mavlink message
// If this list is updated add default setup for this param in activity DEFAULT_ACTIVITY(0)
extern volatile bool allowed_params_inited;
extern AllowedParam ALLOWED_PARAMS[ALLOWED_PARAM_COUNT];
bool init_allowed_params();

}
// end of namespace Activity
