#pragma once

#include "allowed_params.hpp"
#include <initializer_list>
#include "activity_lib_constants.h"

// These activities are predefined and should be referenced in all corresponding lists
enum
{
    ACTIVITY_TEST,
    ACTIVITY_SURF,
    ACTIVITY_SKI,
    ACTIVITY_SKATE,
    ACTIVITY_CYCLE,
    ACTIVITY_WAKE,
    ACTIVITY_BIKE,
    ACTIVITY_SNOWBOARD,
    ACTIVITY_KITE,
    ACTIVITY_CUSTOM,
    ACTIVITY_MAX
};

extern const char* activityNames[];


namespace Activity {

class __EXPORT ParamConfig {
    public:

        int p_id; 
        int limit_kind;
        float istart;
        float iend;
        float step;
        int value_num;
        float * values = nullptr;

        float default_value;

        ParamConfig ();
        ParamConfig(const char * _name, int _limit_kind, int _target_device, float _default_value, float _istart, float _iend, float _step, std::initializer_list <float> _values);
        ~ParamConfig();
        ParamConfig& operator = (const ParamConfig &a);
        ParamConfig(const ParamConfig &a);
        const char * get_name();
        param_target_device get_target_device();
        const char * get_param_name();
        const char * get_display_name();

};

class __EXPORT ActivityConfig
{
    public:
        int id;
        const char * name;
        int param_count;
        ParamConfig * params;

        ActivityConfig();
        ActivityConfig(int _param_count);

        ActivityConfig(int _id, const char * _name, std::initializer_list <ParamConfig> _params );
        ~ActivityConfig();

        ActivityConfig& operator=(ActivityConfig &&a);

    private:
        ActivityConfig(const ActivityConfig &a);
        ActivityConfig& operator=(const ActivityConfig &a);

}; 

extern bool activity_config_list_inited;
extern ActivityConfig ACTIVITY_CONFIG_LIST[ ACTIVITIES_COUNT+1 ];
extern ActivityConfig ACTIVITY_CONFIG_BASE;

int init_activity_config_list();
ParamConfig * get_activity_param_config(int activity, int param);

// End of Activity namespace 
}
