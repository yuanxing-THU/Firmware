#pragma once

#include <systemlib/systemlib.h>

#include <uORB/topics/vehicle_status.h>

#include "protocol.h"

#include "request_base.hpp"
#include "uorb_base.hpp"

template <>
struct Request< CMD_REBOOT >
{
	using value_type = void;
};

inline errcode_t
verify_request(Request< CMD_REBOOT >)
{
	Subscription<vehicle_status_s, ORB_ID(vehicle_status)> s;
	auto status = orb_read(s);

	bool ok = status.airdog_state == AIRD_STATE_STANDBY;
	return ok ? ERRCODE_OK : ERRCODE_REQUEST_FORBIDDEN;
}

template <typename Device>
void __attribute__(( noreturn ))
reply(Request< CMD_REBOOT >, Device & dev)
{
	/*
	 * Reply is header only. Let it be delivered.
	 */
	usleep(100 * 1000 /*µs*/);

	systemreset(false);
}

