#pragma once

#include <systemlib/param/param.h>

namespace Activity {

class __EXPORT DogActivityManager {

    public:

        DogActivityManager();

        ~DogActivityManager();
        
        bool init();

        bool is_inited();

        bool set_activity(uint8_t activity);

        bool check_incoming_params();

    private:

        bool process_incoming_params();

        bool send_params_to_leash();

        bool apply_params();

        bool process_virtual_params();

        param_t _param_activity;

        bool _file_state_ok;

        uint8_t _current_activity;

        bool _inited;

        uint64_t last_file_state_check_time = 0;

        int _incoming_activity_params_orb;
    
};
}
