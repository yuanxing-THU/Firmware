#pragma once

#include <airdog/hwinfo/version.hpp>
#include <airdog/hwinfo/stamp.hpp>
#include <stm32f4/flash.hpp>

#include "request_base.hpp"

template <>
struct Request< CMD_HANDSHAKE >
{
	using value_type = void;
};

template <typename Device>
void
reply(Request< CMD_HANDSHAKE >, Device & dev)
{
	HandshakeReply buf { 1, 0 };
	write(dev, &buf, sizeof buf);
}

template <>
struct Request< CMD_VERSION_FIRMWARE >
{
	using value_type = void;
};

template <typename Device>
void
reply(Request< CMD_VERSION_FIRMWARE >, Device & dev)
{
	VersionFirmwareReply r{ 0, 0 };
	write(dev, &r, sizeof r);
}

template <>
struct Request< CMD_VERSION_HARDWARE >
{
	using value_type = void;
};

errcode_t
verify_request(Request< CMD_VERSION_HARDWARE >)
{
	using namespace AirDog::HardwareInfo;
	bool ok = otp_is_valid();
	return ok ? ERRCODE_OK : ERRCODE_OTP_ERROR;
}

template <typename Device>
void
reply(Request< CMD_VERSION_HARDWARE >, Device & dev)
{
	using namespace AirDog::HardwareInfo;
	uint32_t v = version_hw();

	VersionHardwareReply r{
		uint8_t(v >> 24), uint8_t(v >> 16), uint16_t(v)
	};
	write(dev, &r, sizeof r);
}

template <>
struct Request< CMD_INFO_SERIAL >
{
	using value_type = void;
};

template <typename Device>
void
reply(Request< CMD_INFO_SERIAL >, Device & dev)
{
	using stm32f4_23::flash::mcu_udid;

	char serial[25];

	/*
	 * Keep memory byte order despite PX4 reverses it.
	 */
	snprintf(serial, sizeof serial, "%08x%08x%08x",
			mcu_udid[0], mcu_udid[1], mcu_udid[2]);

	write(dev, serial, 24);
}
