#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <uORB/uORB.h>
#include <uORB/topics/activity_params.h>

#include "allowed_params.hpp"

#include "activity_lib_constants.h"
#include "activity_files.hpp"

#include <drivers/drv_hrt.h>

#include "dog_activity_manager.hpp"


namespace Activity {

DogActivityManager::DogActivityManager(){

    _received_activity_params_sub = orb_subscribe(ORB_ID(activity_params));

    _file_state_ok = false;
    _inited = false;
    _current_activity = 0;
    _activity_params_sndr_pub = 0;

    _last_params_received_process = 0;

    init_allowed_params();

    Activity::Files::clear_file_state();

    param_get(param_find("A_ACTIVITY"), &(_current_activity));

}

DogActivityManager::~DogActivityManager() {

    orb_unsubscribe(_received_activity_params_sub);
}

bool
DogActivityManager::init() {

    if (_inited)
        return true;

    check_file_state();

    if (_file_state_ok) {
        if (set_activity(_current_activity))
            _inited = true;
    }

    return _inited;

}

bool
DogActivityManager::check_file_state(){

    if (hrt_absolute_time() - last_file_state_check_time > 1000000) {

        last_file_state_check_time = hrt_absolute_time();

        printf("Checking files.. \n");

        if (Activity::Files::get_file_state()) {

            printf("Ok file present.\n");
            _file_state_ok = true;

        } else {

            printf("No ok file. Rechecking status.\n");

            _file_state_ok = Activity::Files::check_file_state();

            if (_file_state_ok) {
                printf("file state ok!\n");
            } else {
                printf("file state bad!\n");
            }
        } 
    }

    if (!_file_state_ok) {
        _inited = false;
    }

    return _file_state_ok;

}

bool 
DogActivityManager::set_activity(int32_t activity){

    printf("Setting activity %d\n", activity);

    _current_activity = activity;

    param_set(param_find("A_ACTIVITY"), &activity);

    if (!Files::activity_file_to_orb(activity))
        return false;

    if (!process_virtual_params())
        return false;

    if (!send_params_to_leash())
         return false;

    if (!apply_params())
        return false;

    return true;

}

bool 
DogActivityManager::check_received_params() {

    activity_params_s activity_params;

    bool updated;
    orb_check(_received_activity_params_sub, &updated);

    if (updated) {

        orb_copy(ORB_ID(activity_params), _received_activity_params_sub, &activity_params);

        if (activity_params.type == ACTIVITY_PARAMS_RECEIVED ) {
            process_received_params();
        }
    }
}

bool 
DogActivityManager::process_received_params(){

    printf("Processing received params\n");

    if (!process_virtual_params())
        return false;

    if (!Files::activity_orb_to_file())
        return false;

    if (!apply_params())
        return false;

    if (!send_params_to_leash())
        return false;

    return true;

}

bool
DogActivityManager::send_params_to_leash(){

    _activity_params_sndr.type = ACTIVITY_PARAMS_MSG_VALUES;

    if (_activity_params_sndr_pub >  0) {
        orb_publish(ORB_ID(activity_params_sndr), _activity_params_sndr_pub, &_activity_params_sndr);
    } else {
        _activity_params_sndr_pub = orb_advertise(ORB_ID(activity_params_sndr), &_activity_params_sndr);
    }

    return true;
}
        
bool
DogActivityManager::process_virtual_params() {

    activity_params_s activity_params_read;
    activity_params_s activity_params_processed;

	int activity_params_sub = orb_subscribe(ORB_ID(activity_params));

    orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params_read);

	orb_unsubscribe(activity_params_sub);

    activity_params_processed.type = ACTIVITY_PARAMS_PROCESSED;

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++)
        activity_params_processed.values[i] = activity_params_read.values[i];

    int activity_params_pub = orb_advertise(ORB_ID(activity_params), &activity_params_processed);

    if (activity_params_pub < 0) {
        printf("Failed to publish reprocessed activity params\n");
        return false;
    }

    return true;
}

bool
DogActivityManager::apply_params() {

	int activity_params_sub = orb_subscribe(ORB_ID(activity_params));

    activity_params_s activity_params;
	orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params);

    orb_unsubscribe(activity_params_sub);

    for (int p=0;p<ALLOWED_PARAM_COUNT;p++) {
        if (ALLOWED_PARAMS[p].target_device == DOG || ALLOWED_PARAMS[p].target_device == ALL) {
          
            printf("Applying param on dog - %s: %.2f\n", ALLOWED_PARAMS[p].name, (double)activity_params.values[p]);
            if (param_set(param_find(ALLOWED_PARAMS[p].name), &activity_params.values[p]) != 0) {
                printf("Param %s connot be found.\n", ALLOWED_PARAMS[p].name);
                return false;
            }  
        }   
    }

    return true;
}

bool
DogActivityManager::is_inited() {

    check_file_state();
    return _inited;

}

}
// End of Activity namespace
