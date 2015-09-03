#pragma once

#include <cstdint>

// forward declarations

namespace stm32f4_23 {}

namespace stm32f42 = stm32f4_23;


// content

namespace stm32f4_23
{
namespace flash
{

using MCU_UDID_t = uint32_t[3];

const MCU_UDID_t & mcu_udid = *reinterpret_cast<const MCU_UDID_t *>(0x1FFF7A10);

//__EXPORT
struct write_guard
{
	write_guard();
	~write_guard();

	write_guard(const write_guard &) = delete;
	write_guard & operator = (const write_guard &) = delete;
};

__EXPORT bool
status_ok();

__EXPORT bool
otp_is_untouched(size_t first, size_t size);

__EXPORT bool
otp_verify_u32(size_t first, const uint32_t value[], size_t n);

__EXPORT bool
otp_write_u32(const write_guard &,
		size_t first, const uint32_t value[], size_t n);

__EXPORT bool
otp_any_byte_has_lock(size_t first_byte, size_t size);

__EXPORT bool
otp_all_bytes_are_locked(size_t first_byte, size_t size);

__EXPORT bool
otp_burn_locks(const write_guard &, size_t first_byte, size_t size);

}
// end of namespace flash
}
// end of namespace stm32f4_23
