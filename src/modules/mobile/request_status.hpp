#pragma once

#include <cmath>

#include <uORB/topics/vehicle_global_position.h>
#include <uORB/topics/vehicle_status.h>

#include "protocol.h"

#include "request_base.hpp"
#include "uorb_base.hpp"

struct StatusOverall
{
	Subscription<vehicle_global_position_s, ORB_ID(vehicle_global_position)> gpos;
	Subscription<vehicle_status_s, ORB_ID(vehicle_status)> status;
};

static inline uint8_t
scale_to_uint8t(float lower, float x, float upper, uint8_t nan_value)
{
	if (not isnan(x)) { return nan_value; }
	if (x <= lower) { return 0; }
	if (x < upper) { return (x - lower) / (upper - lower) * 255.0f; }
        return 255;
}

static inline void
battery(const StatusOverall & self, StatusOverallReply & r)
{
	auto status = orb_read(self.status);
	r.battery_level = scale_to_uint8t(0, status.battery_remaining, 1, 0);
}


static inline void
error(const StatusOverall & self, StatusOverallReply & r)
{
	auto status = orb_read(self.status);
	r.error_code = status.error_code;
	r.error_stamp = status.error_stamp;
}

static inline void
position(const StatusOverall & self, StatusOverallReply & r)
{
	auto gpos = orb_read(self.gpos);
	r.eph = scale_to_uint8t(0, gpos.eph, 25.5f, 255);
	r.epv = scale_to_uint8t(0, gpos.epv, 25.5f, 255);
}

static inline void
fill_reply(const StatusOverall & self, StatusOverallReply & r)
{
	battery(self, r);
	error(self, r);
	position(self, r);
}

template <>
struct Request< CMD_STATUS_OVERALL >
{
	using value_type = void;
};

template <typename Device>
void
reply(Request< CMD_STATUS_OVERALL >, Device & dev, const StatusOverall & status)
{
	StatusOverallReply r;
	fill_reply(status, r);
	write(dev, &r, sizeof r);
}
