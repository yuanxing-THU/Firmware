#include "activity_change_manager.hpp"
#include "activity_lib_constants.h"

#include <systemlib/param/param.h>
#include <string.h>

namespace Activity {

ActivityChangeManager::ActivityChangeManager(int _a_id, bool _test_mode){

        a_id = _a_id;
        test_mode = _test_mode;

        params_inited = false;
        params_up_to_date = false;
        param_count = ALLOWED_PARAM_COUNT;


        activity_params_sub = orb_subscribe(ORB_ID(activity_params));

        for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {

            params[i].p_id = i;
            params[i].limits = nullptr;

            for (int j=0;j<ACTIVITY_LIMITS_LIST[_a_id].param_count;j++)
                if (ACTIVITY_LIMITS_LIST[0].params[i].p_id == ACTIVITY_LIMITS_LIST[_a_id].params[j].p_id) {
                    params[i].limits = &ACTIVITY_LIMITS_LIST[_a_id].params[j];
                }

            if (params[i].limits == nullptr) {
                params[i].limits = &ACTIVITY_LIMITS_LIST[0].params[i];
            }

        }
}

ActivityChangeManager::~ActivityChangeManager() {
   orb_unsubscribe(activity_params_sub);
}

bool
ParamChangeManager::get_param_name(char * buffer, const int buffer_len){
    strncpy ( buffer, ALLOWED_PARAMS[p_id].name, strlen(ALLOWED_PARAMS[p_id].name)+1 );
    return true; 
}

bool
ParamChangeManager::get_display_name(char * buffer, const int buffer_len){
    strncpy ( buffer, ALLOWED_PARAMS[p_id].display_name, strlen(ALLOWED_PARAMS[p_id].display_name)+1 );
    return true; 
}

bool
ParamChangeManager::get_display_value(char * buffer, const int buffer_len){

    switch (limits->type){
        case PARAM_LIMIT_TYPE::VALUES_INT: 
            snprintf(buffer, buffer_len, "%i", (int)value);
            return true;
        case PARAM_LIMIT_TYPE::INTERVAL:
        case PARAM_LIMIT_TYPE::VALUES_FLOAT:
            snprintf(buffer, buffer_len, "%.2f", (double)value);
            return true;
        case PARAM_LIMIT_TYPE::VALUES_STR:

            if ( !(value >= 0 && value <= ALLOWED_PARAMS[p_id].display_value_count) ) {
               printf("Value %i out of range.\n", (int)value);
               strncpy(buffer, "No value", buffer_len);
            } else {
                strncpy(buffer, ALLOWED_PARAMS[p_id].display_values[(int)value], buffer_len);
            }
            return true;
        default:
            strncpy(buffer, "No such type", buffer_len);
            return false;
    }
}

int 
ParamChangeManager::move_value(int step_dir){

    bool found = false;
    int ind = 0;

    float eps = 1e-6;

    for (;ind < (limits->value_num); ind++ ) {
        if (float_eq(value, limits->values[ind]) && value -eps < limits->values[ind]) {
            found = true;
            break;
        }
    }

    if (found == false)
        ind = 0;
    else 
        ind = ind+step_dir;

    if (ind < 0) ind += limits->value_num;
    if (ind >= limits->value_num) ind -= limits->value_num;

    value = (limits->values[ind]);

    return 0;
}


int 
ParamChangeManager::move_interval(int step_dir){

    float istart = (limits->istart);
    float iend = (limits->iend);
    float step = (limits->step);

    if (value < istart) 
        value = istart;
    else 
    if (value > iend) 
        value = iend;
    else 
        {
            if (float_eq(value, iend) && step_dir == 1)
                value = istart;
            else
            if (float_eq(value, istart) && step_dir == -1) 
                value = iend;
            else 
                {

                    float tmp_val = istart;
                    float next_tmp_val;

                    for (;tmp_val<=iend;tmp_val+=step) {
                        
                        next_tmp_val = tmp_val + step;

                        if (step_dir == 1) {
                            if (value >= tmp_val  && value < next_tmp_val) {
                                value = next_tmp_val;
                                break;
                            }
                        }

                        if (step_dir == -1) {
                            if (value > tmp_val  && value <= next_tmp_val) {
                                value = tmp_val;
                                break;
                            }
                        }
                    }
                }
        }
    return 0;
}


bool
ParamChangeManager::get_next_value(char * buffer, int buffer_len){

    switch ( limits->type ){
        case PARAM_LIMIT_TYPE::INTERVAL:
            move_interval(1);
        break;
        case PARAM_LIMIT_TYPE::VALUES_INT: 
        case PARAM_LIMIT_TYPE::VALUES_FLOAT:
        case PARAM_LIMIT_TYPE::VALUES_STR:
            move_value(1);
        break;
    }

    return get_display_value(buffer, buffer_len);
}

bool
ParamChangeManager::get_prev_value(char * buffer, int buffer_len){

    switch ( limits->type ){
        case PARAM_LIMIT_TYPE::INTERVAL:
            move_interval(-1);
        break;
        case PARAM_LIMIT_TYPE::VALUES_INT: 
        case PARAM_LIMIT_TYPE::VALUES_FLOAT:
        case PARAM_LIMIT_TYPE::VALUES_STR:
            move_value(-1);
        break;
    }

    return get_display_value(buffer, buffer_len);
}

int
ParamChangeManager::save_value(){

    saved_value = value;
    return 0;
}

int
ParamChangeManager::cancel_value(){
    value = saved_value;
    return 0;
}

int
ParamChangeManager::get_id(){
    return p_id;
}

bool
ActivityChangeManager::get_display_name(char * buffer, const int buffer_len){
   snprintf(buffer, buffer_len, "%s", ACTIVITY_LIMITS_LIST[a_id].name); 
   return true; 
}

ParamChangeManager *
ActivityChangeManager::get_next_visible_param(){

    int it = 0;
    while (true) {

        cur_param_id++;

        if (cur_param_id == ALLOWED_PARAM_COUNT)
            cur_param_id = 0;

        if (it>=ALLOWED_PARAM_COUNT)
            break;

        if (params[cur_param_id].limits->type != PARAM_LIMIT_TYPE::INVISIBLE) {
            break;
        }

        it++;
    }

    return &params[cur_param_id];
}


ParamChangeManager *
ActivityChangeManager::get_current_param(){

    int it = 0;
    while (true) {

        if (params[cur_param_id].limits->type != PARAM_LIMIT_TYPE::INVISIBLE) {
            break;
        }

        cur_param_id++;

        if (cur_param_id == ALLOWED_PARAM_COUNT)
            cur_param_id = 0;

        if (it>=ALLOWED_PARAM_COUNT)
            break;

        it++;
    }

    return &params[cur_param_id];
}

ParamChangeManager *
ActivityChangeManager::get_prev_visible_param(){

    int it = 0;
    while (true) {

        cur_param_id--;

        if (cur_param_id < 0)
            cur_param_id = ALLOWED_PARAM_COUNT-1;

        if (it >= ALLOWED_PARAM_COUNT)
            break;

        if (params[cur_param_id].limits->type != PARAM_LIMIT_TYPE::INVISIBLE) {
            break;
        }

        it++;
    }

    return &params[cur_param_id];
}

bool
ActivityChangeManager::init_params(activity_params_s activity_params){

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {
        params[i].saved_value = activity_params.values[i]; 
        params[i].value = activity_params.values[i]; 

        if (ALLOWED_PARAMS[i].target_device == ALL || ALLOWED_PARAMS[i].target_device == LEASH) {

            if (param_set(param_find(ALLOWED_PARAMS[i].name), &activity_params.values[i]) != 0) {
                return false;
            }

        }
    }

    params_inited = true;
    cur_param_id = 0;
    return true;
}

bool
ActivityChangeManager::save_params(){

    activity_params_s activity_params;

    activity_params.type = ACTIVITY_PARAMS_SAVED;

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {
        activity_params.values[i] = params[i].saved_value;
    }

    int activity_params_pub = orb_advertise(ORB_ID(activity_params), &activity_params);

    if (activity_params_pub <= 0) {
        printf("Failed to publish activity params to orb.\n");
        return false;
    }

    send_params_to_dog();

    return true;

}

bool
ActivityChangeManager::send_params_to_dog(){

    activity_params_sndr_s activity_params_sndr;
    activity_params_sndr.type = ACTIVITY_PARAMS_MSG_VALUES;

    int activity_params_sndr_pub = orb_advertise(ORB_ID(activity_params_sndr), &activity_params_sndr);

    if (activity_params_sndr_pub <= 0){
        printf("Failed to publish activity params sender to orb.\n");
        return false;
    }

    params_up_to_date = false;
}

int
ActivityChangeManager::cancel_params(){
    params_inited = false;
}

bool 
ActivityChangeManager::params_updated() {

    if (params_up_to_date)
        return true;
    else  {
    
        bool updated = false;
        orb_check(activity_params_sub, &updated);

        if (updated) {

            activity_params_s activity_params;
            orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params);

            if (activity_params.type == ACTIVITY_PARAMS_RECEIVED) {
                init_params(activity_params);
                params_up_to_date = true;
            }
        }
    }

    return params_up_to_date;

}

__EXPORT ActivityChangeManager 
getActivityChangeManager(int activity_id, bool test_mode){

    init_activity_limits_list();

    activity_request_sndr_s activity_request_sndr;
    activity_request_sndr.type = ACTIVITY_REQUEST_PARAMS; 

    int activity_request_sndr_pub = orb_advertise(ORB_ID(activity_request_sndr), &activity_request_sndr);

    if (activity_request_sndr_pub <= 0) {
        // TODO: Do something
    }

    ActivityChangeManager mgr = ActivityChangeManager(activity_id, test_mode);
    return mgr;
}

bool
float_eq(float a, float b) {
    float eps = 1e-6;
    return (a+eps > b && a-eps < b);
}

}
// End of Activity namespace
