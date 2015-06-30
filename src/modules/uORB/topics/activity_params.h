/**
 * @file activity_params.h
 * Definition of the activity_params uORB topic.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../uORB.h"

typedef enum { 
    ACTIVITY_PARAMS_RECEIVED = 0,
    ACTIVITY_PARAMS_SAVED,
    ACTIVITY_PARAMS_PROCESSED,
    ACTIVITY_PARAMS_SET_FROM_FILE,
    ACTIVITY_PARAMS_ERROR
} activity_params_type;

struct activity_params_s {
    activity_params_type type;
    float values[32];
};

ORB_DECLARE(activity_params);


typedef enum {
    ACTIVITY_PARAMS_MSG_VALUES = 0,
    ACTIVITY_PARAMS_MSG_FAILED,
} activity_params_msg_type;

struct activity_params_sndr_s {
    activity_params_msg_type type;
};

ORB_DECLARE(activity_params_sndr);

typedef enum {
    ACTIVITY_REQUEST_PARAMS = 0,
    ACTIVITY_REQUEST_FAILED
} activity_request_type;

struct activity_request_sndr_s {
    activity_request_type type;
};

ORB_DECLARE(activity_request_sndr);
