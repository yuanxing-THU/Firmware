#include "activity_limits_list.hpp"
#include "uORB/topics/activity_params.h"

namespace Activity {

class __EXPORT ParamChangeManager {

    public:

        bool get_param_name(char* buffer, int buffer_len);
        bool get_display_name(char* buffer, int buffer_len);
        bool get_display_value(char* buffer, int buffer_len);

        bool get_next_value(char* buffer, int buffer_len);
        bool get_prev_value(char* buffer, int buffer_len);

        int save_value();
        int cancel_value();

        int get_id();

        // TODO: move those to private and handle trough constructor
        int p_id;
        ParamLimits * limits; 

        // TODO: move those to private and handle trough functions
        float value;
        float saved_value;

    private:

        int move_value(int step_dir);
        int move_interval(int step_dir);
};

class __EXPORT ActivityChangeManager {

    public:

        bool get_display_name(char* buffer, int buffer_len);

        ParamChangeManager * get_next_visible_param();
        ParamChangeManager * get_prev_visible_param();
        ParamChangeManager * get_current_param();
        
        bool save_params();
        int cancel_params();

        bool init_params(activity_params_s);
        bool params_updated();

        bool send_params_to_dog();

        ActivityChangeManager(int _a_id, bool _test_mode);
        ~ActivityChangeManager();
       
    private:
        int a_id;
        int param_count;

        ParamChangeManager params[ALLOWED_PARAM_COUNT];
        
        int cur_param_id;

        bool params_inited;
        bool params_up_to_date; 

        int activity_params_sub;

        bool test_mode;
};

__EXPORT ActivityChangeManager 
getActivityChangeManager(int activity_id, bool test_mode = false);

__EXPORT bool
float_eq(float a, float b);

bool
publish_fake_activity_params();

}
// End of namespace Activity
