#pragma once
#include <stdint.h>
#include <sys/types.h>
#include "../uORB.h"

struct follow_path_data_s {

    float dst_i;
    float dst_p;
    float dst_d;

    float vel;
    float point_count;
    
    float dst_to_gate;
    float dst_to_tunnel_middle;

    float fx;
    float fy;
    float fz;

    float sx;
    float sy;
    float sz;
};

ORB_DECLARE(follow_path_data);
