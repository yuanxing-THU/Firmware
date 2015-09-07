#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#define PACKED __attribute__((__packed__))

namespace AirDog
{
namespace HardwareInfo
{

/*
 * Types
 */
union board_id_t
{
	uint32_t items_u32[4];
	struct {
		uint32_t mcu_serial[3];
		uint32_t version_hw;
	} contents;

	static_assert(sizeof items_u32 == sizeof contents,
			"Invalid board_id_t definition.");
};

using ECDSA_t = uint32_t[16];

struct PACKED header_v1
{
	static constexpr unsigned HEADER_VERSION = 1;
	uint32_t version_hw;
	uint32_t sign_key_id;
};

using header_current_t = header_v1;

struct PACKED FactoryStamp
{
	static constexpr unsigned
	VERSION_TAG_CURRENT = header_current_t::HEADER_VERSION;

	static constexpr uint16_t
	MAGIC = 0x0441;

	uint32_t crc32;
	uint16_t magic;
	uint16_t version_tag;
	union {
		header_v1 v1;
		uint8_t raw[24];
	} header;
	ECDSA_t sign;
};

static_assert(std::is_pod<FactoryStamp>::value,
	"FactoryStamp must be POD type.");
#ifndef offsetof
# define offsetof(type,member) ((std::size_t) &(((type*)0)->member))
#endif
static_assert(offsetof(FactoryStamp, sign) == 32,
	"FactoryStamp::signature must align to next separate OTP block.");

/*
 * Functions
 */

__EXPORT void
fill_mcu_serial(board_id_t & board_id);

__EXPORT void
fill(board_id_t & board_id);

__EXPORT uint32_t
signature_key_id();

__EXPORT void
signature_copy(uint8_t (&)[64]);

}
// end of namespace HardwareInfo
}
// end of namespace AirDog
