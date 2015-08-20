#pragma once

#include <systemlib/param/param.h>
#include <uORB/topics/activity_params.h>

namespace Activity {

class __EXPORT DogActivityManager {

    public:

        DogActivityManager();

        ~DogActivityManager();
        
        bool init();

        bool is_inited();

        bool set_activity(int32_t activity);

        bool check_received_params();

        bool check_file_state();

    private:

        bool process_received_params();

        bool send_params_to_leash();

        bool apply_params();

        bool process_virtual_params();

        param_t _param_activity;

        bool _file_state_ok;

        int32_t _current_activity;

        bool _inited;

        uint64_t last_file_state_check_time = 0;

        int _received_activity_params_sub;

        int64_t _last_params_received_process;

        activity_params_sndr_s _activity_params_sndr;
        
        int _activity_params_sndr_pub;
    
};
}
