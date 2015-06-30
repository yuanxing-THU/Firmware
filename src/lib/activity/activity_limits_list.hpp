#pragma once

#include "allowed_params.hpp"
#include <initializer_list>
#include "activity_lib_constants.h"
#include "activity_limits_list.hpp"

namespace Activity {


class __EXPORT ParamLimits {
    public:

        int p_id; 
        int type;
        float istart;
        float iend;
        float step;
        int value_num;
        float * values = nullptr;

        ParamLimits ();
        ParamLimits(const char * _name, int _type, int _target_device, float _istart, float _iend, float _step, std::initializer_list <float> _values);
        ~ParamLimits();
        ParamLimits& operator = (const ParamLimits &a);
        ParamLimits(const ParamLimits &a);
        const char * get_name();
        param_target_device get_target_device();
        const char * get_param_name();
        const char * get_display_name();

};

class __EXPORT ActivityLimits
{
    public:
        int id;
        const char * name;
        int param_count;
        ParamLimits * params;

        ActivityLimits();
        ActivityLimits(int _param_count);

        ActivityLimits(int _id, const char * _name, std::initializer_list <ParamLimits> _params );
        ~ActivityLimits();

        ActivityLimits& operator=(ActivityLimits &&a);

    private:
        ActivityLimits(const ActivityLimits &a);
        ActivityLimits& operator=(const ActivityLimits &a);

}; 

extern bool activity_limits_list_inited;
extern ActivityLimits ACTIVITY_LIMITS_LIST[ ACTIVITIES_COUNT+1 ];

int init_activity_limits_list();

// End of Activity namespace 
}
