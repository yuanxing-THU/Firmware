#include <stdio.h>

#include <uORB/uORB.h>
#include <uORB/topics/activity_params.h>

#include "allowed_params.hpp"

#include "activity_lib_constants.h"
#include "activity_file_manager.hpp"

#include <drivers/drv_hrt.h>

#include "dog_activity_manager.hpp"

namespace Activity {

DogActivityManager::DogActivityManager(){

    init_allowed_params();

    _file_state_ok = false;
    _inited = false;
    _current_activity = 0;

    param_get(param_find("A_ACTIVITY"), &(_current_activity));
    init();

}

DogActivityManager::~DogActivityManager() {
}

bool
DogActivityManager::init() {

    if (_inited)
        return true;

    if (!_file_state_ok && hrt_absolute_time() - last_file_state_check_time > 1000000) {

        printf("Checking files..");
        _file_state_ok = Activity::Files::get_file_state();
        last_file_state_check_time = hrt_absolute_time();
    }

    if (_file_state_ok) {
        if (set_activity(_current_activity))
            _inited = true;
    }

    return _inited;

}


bool 
DogActivityManager::set_activity(uint8_t activity){

    _current_activity = activity;

    param_set(param_find("A_ACTIVITY"), &activity);
    Files::activity_file_to_orb(activity);

    process_virtual_params();
    send_params_to_leash();
    apply_params();

}

bool 
DogActivityManager::check_incoming_params() {

    bool params_updated;

    orb_check(_incoming_activity_params_orb, &params_updated);

    if (params_updated) {

        activity_params_s activity_params;
        orb_copy(ORB_ID(activity_params), _incoming_activity_params_orb, &activity_params);

        if (activity_params.type == ACTIVITY_PARAMS_RECEIVED )
            process_incoming_params();
    }
}

bool 
DogActivityManager::process_incoming_params(){

    process_virtual_params();
    send_params_to_leash();
    apply_params();

}

bool
DogActivityManager::send_params_to_leash(){

    activity_params_sndr_s activity_params_sndr;
    activity_params_sndr.type = ACTIVITY_PARAMS_MSG_VALUES;

    int activity_params_sndr_pub = orb_advertise(ORB_ID(activity_params_sndr), &activity_params_sndr);

    if (activity_params_sndr_pub <= 0) {
        printf("Failed to publish activity sender.");
        return false;
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

    for (int p=0;p<ALLOWED_PARAM_COUNT;p++) {
        if (ALLOWED_PARAMS[p].target_device == DOG || ALLOWED_PARAMS[p].target_device == ALL) {
          
            printf("%s\n", ALLOWED_PARAMS[p].name);
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
    return _inited;
}

}
// End of Activity namespace
