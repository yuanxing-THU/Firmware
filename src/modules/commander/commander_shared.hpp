#ifndef __COMMANDER_SHARED_HPP_INCLUDED__
#define __COMMANDER_SHARED_HPP_INCLUDED__

#include <uORB/topics/vehicle_command.h>

__EXPORT void commander_shared_preprocess_vehicle_command(struct vehicle_command_s * cmd);

#endif
