#include <cstring>

#include <crc32.h>

#include <systemlib/otp.h>

#include <stm32f4/flash.hpp>

#include "stamp.hpp"
#include "version.hpp"

namespace AirDog
{
namespace HardwareInfo
{

auto & factory_stamp = *(const FactoryStamp *)ADDR_OTP_START;

bool
otp_is_valid()
{
	auto p = (const uint8_t *)&factory_stamp;
	uint32_t checksum = crc32(p + 4, sizeof factory_stamp - 4);

	if (checksum != factory_stamp.crc32) { return false; }

	if (factory_stamp.magic != FactoryStamp::MAGIC) { return false; }

	switch (factory_stamp.version_tag)
	{
	case 1:
		return true;
	default:
		return false;
	}
}

uint32_t
version_otp() { return factory_stamp.version_tag; }

uint32_t
version_hw()
{
	switch (factory_stamp.version_tag)
	{
	case 1:
		return factory_stamp.header.v1.version_hw;
	default:
		return 0xFFffFFff; /* let it look like uninitialized OTP; */
	}
}

uint32_t
signature_key_id()
{
	switch (factory_stamp.version_tag)
	{
	case 1:
		return factory_stamp.header.v1.sign_key_id;
	default:
		return 0xFFffFFff; /* let it look like uninitialized OTP; */
	}
}

void
signature_copy(uint8_t (&sig)[64])
{
	static_assert(sizeof sig == sizeof factory_stamp.sign,
			"Signature size mismatch.");
	memcpy(sig, factory_stamp.sign, sizeof sig);
}

void
fill_mcu_serial(board_id_t & board_id)
{
	namespace F = stm32f4_23::flash;

	static_assert(
		sizeof board_id.contents.mcu_serial == sizeof F::mcu_udid,
		"MCU serial size mismatch."
	);

	/*
	 * Keep memory byte order despite PX4 reverses it.
	 */
	board_id.contents.mcu_serial[0] = F::mcu_udid[0];
	board_id.contents.mcu_serial[1] = F::mcu_udid[1];
	board_id.contents.mcu_serial[2] = F::mcu_udid[2];
}

void
fill(board_id_t & board_id)
{
	fill_mcu_serial(board_id);
	board_id.contents.version_hw = version_hw();
}

}
// end of namespace HardwareInfo
}
// end of namespace AirDog
