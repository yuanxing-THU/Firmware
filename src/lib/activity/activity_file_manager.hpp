#pragma once

#include <limits.h>

#include <cstdint>

#include "activity_lib_constants.h"

namespace Activity
{
namespace Files
{

    // Inlines
    static inline bool
    has_valid_id(uint8_t activity, uint8_t attribute)
    { return (activity < ACTIVITIES_COUNT) and (attribute == 0); }


    // Exported functions
    __EXPORT bool
    get_path(uint8_t activity, uint8_t attribute, char (&pathname)[PATH_MAX]);

    __EXPORT bool
    has_valid_content(uint8_t activity, uint8_t attribute, const char pathname[]);

    __EXPORT bool
    update_activity(uint8_t activity, uint8_t attribute);
    
    __EXPORT bool
    activity_file_to_orb(uint8_t activity);

    __EXPORT bool
    activity_orb_to_file(uint8_t activity);

    __EXPORT bool
    get_file_state();

    __EXPORT bool
    set_file_state();



    // Private functions
    bool
    full_file_state_check();
    
    bool
    validate_activity_params_file(uint8_t activity, const char pathname[]);

    bool
    parse_activity_file_line(char line[],char (&id)[MAX_STR_LEN], char (&value)[MAX_STR_LEN]);

    bool
    create_file_structure();

    bool
    isdigit(char c);

}
// end of namespace Files
}
// end of namespace Activity
