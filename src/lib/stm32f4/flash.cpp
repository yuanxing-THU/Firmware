#include <cstring>

#include <systemlib/otp.h>

#include "flash.hpp"
#include "flash.h"

void flash_getSerial(unsigned char serial[12])
{
	memcpy(serial, &stm32f42::flash::mcu_udid, sizeof(stm32f42::flash::mcu_udid));
}

namespace stm32f4_23
{
namespace flash
{

__EXPORT write_guard::
write_guard()
{
	sched_lock();

	F_unlock();

	FLASH->control |= F_CR_PG;

}

__EXPORT write_guard::
~write_guard()
{
	FLASH->control &= (~F_CR_PG);

	F_lock();

	sched_unlock();
}

bool
status_ok() { return (FLASH->status & ~1u) == 0; }

static void
busy_wait() { while (FLASH->status & F_BSY); }

static bool
verify_u32(const uint32_t area[], const uint32_t value[], size_t n)
{
	size_t i = 0;
	while (i < n and area[i] == value[i]) { ++i; }
	return i == n;
}

static bool
write_u32(
	const write_guard &,
	uint32_t area[],
	const uint32_t value[],
	size_t n
) {
	if (not status_ok()) { return false; }

	FLASH->control = (FLASH->control & CR_PSIZE_MASK) | F_PSIZE_WORD;

	for (size_t i = 0; i < n; ++i)
	{
		area[i] = value[i];
		busy_wait();
		if (not status_ok()) { break; }
	}

	return status_ok();
}

bool
flash_is_erased_u32(const uint32_t area[], size_t n)
{
	size_t i = 0;
	while (i < n and area[i] == 0xFFffFFff) { ++i; }
	return i == n;
}

bool
otp_is_untouched(size_t first, size_t size)
{
	auto area = reinterpret_cast<uint32_t *>(ADDR_OTP_START) + first / 4;
	return flash_is_erased_u32(area, (size + 3) / 4);
}

bool
otp_verify_u32(size_t first, const uint32_t value[], size_t n)
{
	auto area = reinterpret_cast<uint32_t *>(ADDR_OTP_START) + first / 4;
	return verify_u32(area, value, n);
}

bool
otp_write_u32(
	const write_guard & guard,
	size_t first,
	const uint32_t value[],
	size_t n
) {
	auto area = reinterpret_cast<uint32_t *>(ADDR_OTP_START) + first / 4;
	return write_u32(guard, area, value, n);
}

bool
otp_any_byte_has_lock(size_t first_byte, size_t size)
{
	auto lock_area = reinterpret_cast<const uint8_t *>(ADDR_OTP_LOCK_START);

	size_t last = (first_byte + size) / 32;
	if (last >= 16) { return false; }

	size_t first = first_byte / 32;

	if (lock_area[first] == 0) { return true; }

	while (first < last and lock_area[first] != 0) { ++first; }
	return first != last;
}

bool
otp_all_bytes_are_locked(size_t first_byte, size_t size)
{
	auto lock_area = reinterpret_cast<const uint8_t *>(ADDR_OTP_LOCK_START);

	size_t last = (first_byte + size) / 32;
	if (last >= 16) { return false; }

	size_t first = first_byte / 32;

	if (lock_area[first] != 0) { return false; }

	while (first < last and lock_area[first] == 0) { ++first; }
	return first == last;
}

bool
otp_burn_locks(const write_guard & guard, size_t first_byte, size_t size)
{
	if (first_byte % 32 != 0 or size % 32 != 0) { return false; }

	auto flash_area = reinterpret_cast<uint32_t *>(ADDR_OTP_LOCK_START);
	auto copy_area = *(struct otp_lock*)flash_area;

	size_t last = (first_byte + size) / 32;
	if (last >= 16) { return false; }

	size_t i = first_byte / 32;
	while (i < last)
	{
		copy_area.lock_bytes[i] = 0;
		++i;
	}

	static_assert(sizeof copy_area % 4 == 0,
			"write_u32 requres u32 sized array.");
	return write_u32(guard, flash_area,
			(uint32_t *)&copy_area, sizeof copy_area / 4);
}

}
// end of namespace flash
}
// end of namespace stm32f4_23
