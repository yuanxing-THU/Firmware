/**
 * @file activity_params.h
 * Definition of the activity_params uORB topic.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <activity/activity_lib_constants.h>
#include "../uORB.h"

typedef enum { 
    ACTIVITY_PARAMS_DEFAULT = 0,
    ACTIVITY_PARAMS_REMOTE,
    ACTIVITY_PARAMS_LOCAL,
    ACTIVITY_PARAMS_REQUEST
} activity_params_type;

struct activity_params_s {
    uint8_t type;
    float values[Activity::ALLOWED_PARAM_COUNT];
};

ORB_DECLARE(activity_params);

typedef enum {
    ACTIVITY_REQUEST_PARAMS = 0,
    ACTIVITY_REQUEST_FAILED
} activity_request_type;

struct activity_request_s {
    activity_request_type type;
};
ORB_DECLARE(activity_request);


struct activity_remote_t_s {
    uint64_t ts; 
};
ORB_DECLARE(activity_remote_t);


struct activity_received_t_s {
    uint64_t ts; 
};
ORB_DECLARE(activity_received_t);
