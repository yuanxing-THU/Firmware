#pragma once

#include "activity_config_list.hpp"
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
        ParamConfig * config; 

        // TODO: move those to private and handle trough functions
        float value;
        float saved_value;

    private:

        int move_value(int step_dir);
        int move_interval(int step_dir);
};

class __EXPORT ActivityChangeManager {

    public:

        ActivityChangeManager();
        ActivityChangeManager(int _activity);
        ~ActivityChangeManager();

        bool get_display_name(char* buffer, int buffer_len);
        int get_current_activity();

        ParamChangeManager * get_next_visible_param();
        ParamChangeManager * get_prev_visible_param();
        ParamChangeManager * get_current_param();
        
        bool save_params();
        int cancel_params();

        bool process_received_params(activity_params_s);
        bool params_received();
        bool set_waiting_for_params();

        bool request_dog_params();

        bool start_activity_param_messasges();
        bool stop_activity_param_messasges();

        void init(int _activity);
       
    private:

        bool init_activity_config(); 
        bool send_params_to_dog();

        ParamChangeManager params[ALLOWED_PARAM_COUNT];

        int activity;
        int param_count;
        int cur_param_id;

        bool params_up_to_date; 
        int activity_params_sub;
};

bool
float_eq(float a, float b);

// End of namespace Activity
}
