#ifndef FOLLOW_OFFSET_H_
#define FOLLOW_OFFSET_H_

#include <stdint.h>
#include <stdbool.h>
#include "../uORB.h"

struct follow_offset_s
{
    float x,y,z;
};

ORB_DECLARE(follow_offset);

#endif
