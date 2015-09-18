#include "activity_change_manager.hpp"
#include "activity_lib_constants.h"

#include <drivers/drv_hrt.h>
#include <quick_log/quick_log.hpp>

#include <systemlib/param/param.h>
#include <string.h>

namespace Activity {

ActivityChangeManager::ActivityChangeManager() :
    activity(-1),
    param_count(ALLOWED_PARAM_COUNT),
    cur_param_id(0),
    params_up_to_date(false),
    activity_params_sub(-1)
{
    if (!allowed_params_inited) init_allowed_params();
    if (!activity_config_list_inited) init_activity_config_list();

    activity_params_sub = orb_subscribe(ORB_ID(activity_params));
}

ActivityChangeManager::~ActivityChangeManager() {
   orb_unsubscribe(activity_params_sub);
}

bool ActivityChangeManager::isUpdateRequired()
{
    bool result = false;

    int remote_sub = orb_subscribe(ORB_ID(activity_remote_t));
    int received_sub = orb_subscribe(ORB_ID(activity_received_t));

    activity_remote_t_s remote;
    activity_received_t_s received;

    orb_copy(ORB_ID(activity_remote_t), remote_sub, &remote);
    orb_copy(ORB_ID(activity_received_t), received_sub, &received);

    result = remote.ts == 0 || received.ts != remote.ts;

    orb_unsubscribe(remote_sub);
    orb_unsubscribe(received_sub);

    return result;
}

void
ActivityChangeManager::init()
{
    cur_param_id = 0;
    params_up_to_date = false;
}

void
ActivityChangeManager::set_waiting_for_params(){
    params_up_to_date = false;
}

int
ActivityChangeManager::getFollowValue()
{
    int result = -1;

    for (int param_idx = 0;param_idx < ALLOWED_PARAM_COUNT; param_idx++){
        if (strncmp(ALLOWED_PARAMS[param_idx].name, "NAV_AFOL_MODE", 20) == 0) {
            result = (int)params[param_idx].saved_value;
        }
    }
    return result;
}

int
ActivityChangeManager::getLandValue()
{

    int result = -1;

    for (int param_idx = 0;param_idx < ALLOWED_PARAM_COUNT; param_idx++){
        if (strncmp(ALLOWED_PARAMS[param_idx].name, "A_BSC_SAF_ACT", 20) == 0) {
            result = (int)params[param_idx].saved_value;
        }
    }
    return result;

}


bool
ParamChangeManager::get_param_name(char * buffer, const int buffer_len){
    strncpy ( buffer, ALLOWED_PARAMS[p_id].name, buffer_len );
    return true; 
}

bool
ParamChangeManager::get_display_name(char * buffer, const int buffer_len){
    strncpy ( buffer, ALLOWED_PARAMS[p_id].display_name, buffer_len );
    return true; 
}

bool
ParamChangeManager::get_display_value(char * buffer, const int buffer_len){

    switch (config->limit_kind){
        case PARAM_LIMIT_KIND::VALUES_INT: 
            snprintf(buffer, buffer_len, "%i %s", (int)value, ALLOWED_PARAMS[p_id].units);
            return true;
        case PARAM_LIMIT_KIND::INTERVAL:
        case PARAM_LIMIT_KIND::VALUES_FLOAT:
            snprintf(buffer, buffer_len, "%6.2f %s", (double)value, ALLOWED_PARAMS[p_id].units);
            return true;
        case PARAM_LIMIT_KIND::INTERVAL_HP:
            snprintf(buffer, buffer_len, "%6.4f %s", (double)value, ALLOWED_PARAMS[p_id].units);
            return true;
        case PARAM_LIMIT_KIND::VALUES_STR:

            if ( !(value >= 0 && value <= ALLOWED_PARAMS[p_id].display_value_count) ) {
               printf("Value %i out of range.\n", (int)value);
               strncpy(buffer, "No value", buffer_len);
            } else {
                strncpy(buffer, ALLOWED_PARAMS[p_id].display_values[(int)value], buffer_len);
            }
            return true;
        default:
            strncpy(buffer, "No such limit kind", buffer_len);
            return false;
    }
}

int 
ParamChangeManager::move_value(int step_dir){

    bool found = false;
    int ind = 0;

    float eps = 1e-6;

    for (;ind < (config->value_num); ind++ ) {
        if (float_eq(value, config->values[ind]) && value -eps < config->values[ind]) {
            found = true;
            break;
        }
    }

    if (found == false)
        ind = 0;
    else 
        ind = ind+step_dir;

    if (ind < 0) ind += config->value_num;
    if (ind >= config->value_num) ind -= config->value_num;

    value = (config->values[ind]);

    return 0;
}


int 
ParamChangeManager::move_interval(int step_dir){

    float istart = (config->istart);
    float iend = (config->iend);
    float step = (config->step);

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

    switch ( config->limit_kind ){
        case PARAM_LIMIT_KIND::INTERVAL:
        case PARAM_LIMIT_KIND::INTERVAL_HP:
            move_interval(1);
        break;
        case PARAM_LIMIT_KIND::VALUES_INT: 
        case PARAM_LIMIT_KIND::VALUES_FLOAT:
        case PARAM_LIMIT_KIND::VALUES_STR:
            move_value(1);
        break;
    }

    return get_display_value(buffer, buffer_len);
}

bool
ParamChangeManager::get_prev_value(char * buffer, int buffer_len){

    switch ( config->limit_kind ){
        case PARAM_LIMIT_KIND::INTERVAL:
        case PARAM_LIMIT_KIND::INTERVAL_HP:
            move_interval(-1);
        break;
        case PARAM_LIMIT_KIND::VALUES_INT: 
        case PARAM_LIMIT_KIND::VALUES_FLOAT:
        case PARAM_LIMIT_KIND::VALUES_STR:
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
    if (activity >= 0 && activity < ACTIVITIES_COUNT) {
        snprintf(buffer, buffer_len, "%s", ACTIVITY_CONFIG_LIST[activity].name);
        return true;
    } else {
        return false;
    }
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

        if (params[cur_param_id].config->limit_kind != PARAM_LIMIT_KIND::INVISIBLE) {
            break;
        }

        it++;
    }

    return &params[cur_param_id];
}


int
ActivityChangeManager::get_current_activity(){
    return activity;
}

ParamChangeManager *
ActivityChangeManager::get_current_param(){

    int it = 0;
    while (true) {

        if (params[cur_param_id].config->limit_kind != PARAM_LIMIT_KIND::INVISIBLE) {
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

        if (cur_param_id < 0) {
            cur_param_id = ALLOWED_PARAM_COUNT-1;
        }

        if (it >= ALLOWED_PARAM_COUNT)
            break;

        if (params[cur_param_id].config->limit_kind != PARAM_LIMIT_KIND::INVISIBLE) {
           break;
         }

        it++;
    }

    return &params[cur_param_id];
}

bool
ActivityChangeManager::process_received_params(activity_params_s activity_params){

    activity = activity_params.values[0];

    init_activity_config();

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {

        params[i].saved_value = activity_params.values[i]; 
        params[i].value = activity_params.values[i]; 

        if (ALLOWED_PARAMS[i].target_device == ALL || ALLOWED_PARAMS[i].target_device == LEASH) {
            const param_t param = param_find(ALLOWED_PARAMS[i].name);
            if ( param != PARAM_INVALID ) {
                const param_type_e ptype = param_type(param);
                if ( ptype == PARAM_TYPE_FLOAT ) {
                    const float value = activity_params.values[i];
                    if ( param_set(param, &value) != 0 ) {
                        QLOG_sprintf("[DAM] Param %s could not be set.\n", ALLOWED_PARAMS[i].name);
                        return false;
                    }
                } else if ( ptype == PARAM_TYPE_INT32 ) {
                    const int32_t value = int32_t(activity_params.values[i]+0.5f);
                    if ( param_set(param, &value) != 0 ) {
                        QLOG_sprintf("[DAM] Param %s could not be set.\n", ALLOWED_PARAMS[i].name);
                        return false;
                    }
                } else {
                    QLOG_sprintf("[DAM] Param %s has bad type.\n", ALLOWED_PARAMS[i].name);
                    return false;
                }
            } else {
                QLOG_sprintf("[DAM] Param %s connot be found.\n", ALLOWED_PARAMS[i].name);
                return false;
            }
        }
    }

    params_up_to_date = true;
    cur_param_id = 0;

    return true;
}

bool
ActivityChangeManager::init_activity_config() {

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {
        params[i].p_id = i;
        params[i].config = get_activity_param_config(activity, i);
    } 

}

bool
ActivityChangeManager::save_params(){

    activity_params_s activity_params;

    activity_params.type = ACTIVITY_PARAMS_LOCAL;

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {
        activity_params.values[i] = params[i].saved_value;
    }

    int activity_params_pub = orb_advertise(ORB_ID(activity_params), &activity_params);

    if (activity_params_pub <= 0) {
        printf("Failed to publish activity params to orb.\n");
        return false;
    }

    return true;
}

int
ActivityChangeManager::cancel_params(){

    activity_params_s activity_params;
    orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params);

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {

        params[i].saved_value = activity_params.values[i]; 
        params[i].value = activity_params.values[i]; 

    }

}

bool 
ActivityChangeManager::params_received() {

    if (params_up_to_date)
        return true;
    else  {
    
        bool updated = false;
        orb_check(activity_params_sub, &updated);

        if (updated || !isUpdateRequired()) {

            activity_params_s activity_params;
            orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params);

            if (activity_params.type == ACTIVITY_PARAMS_REMOTE) {
                process_received_params(activity_params);
                params_up_to_date = true;
            }
        }
    }

    return params_up_to_date;
}

bool
float_eq(float a, float b) {
    float eps = 1e-6;
    return (a+eps > b && a-eps < b);
}

}
// End of Activity namespace
