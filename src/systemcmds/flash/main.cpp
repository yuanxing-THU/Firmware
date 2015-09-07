#include <nuttx/config.h>

extern "C" __EXPORT
int main(int, const char * const []);

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <crc32.h>

#include <board_config.h>
#include <systemlib/otp.h>

#include <airdog/hwinfo/version.hpp>
#include <airdog/hwinfo/stamp.hpp>
#include <stm32f4/flash.hpp>

#define dbg(...) DOG_PRINT(__VA_ARGS__)

using namespace AirDog::HardwareInfo;

namespace
{

inline bool
streq(const char a[], const char b[]) { return strcmp(a, b) == 0; }

bool
parse_uint(const char s[], uint32_t &n, const char * & tail)
{
	char *p;
	n = std::strtoul(s, &p, 0);
	tail = p;
	return tail != s;
}

bool
parse_uint(const char s[], uint32_t &n)
{
	const char * tail = nullptr;
	bool ok = parse_uint(s, n, tail) and tail and *tail == '\0';
	if (not ok) { dbg("parse_uint('%s') failed.\n", s); }
	return ok;
}

void
print_hex(FILE * f, uint8_t x) { fprintf(f, "%02x", unsigned(x)); }

void
print_hex(FILE * f, uint16_t x) { fprintf(f, "%04x", unsigned(x)); }

void
print_hex(FILE * f, uint32_t x) { fprintf(f, "%08x", unsigned(x)); }

template <typename U>
void
dump_mem(FILE * f, const volatile U * addr, size_t n)
{
	print_hex(f, *addr);
	while (--n)
	{
		fputc(' ', f);

		++addr;
		print_hex(f, *addr);
	}

	fputc('\n', f);
	fflush(f);
}

unsigned
decode_hexdigit(uint8_t x)
{
	if ('0' <= x and x <= '9')
		return x - '0';
	x |= 32;
	if ('a' <= x and x <= 'f')
		return x - 'a' + 10;
	dbg("decode_hexdigit error at '%c'.\n", x);
	return 0xFF;
}

size_t
decode_hex(const char str[], uint8_t buf[], size_t max_len)
{
	const char *s = str;
	size_t i = 0;
	while (i < max_len and *s)
	{
		unsigned l, h;
		h = decode_hexdigit(*s);
		if (h > 16)
		{
			dbg("decode_hexdigit failed at %u '%c'.\n",
				s - str, *s);
			return 0;
		}
		++s;
		l = decode_hexdigit(*s);
		if (l > 16)
		{
			dbg("decode_hexdigit failed at %u '%c'.\n",
				s - str, *s);
			return 0;
		}
		++s;
		buf[i] = (h << 4) + l;
		++i;
	}
	return i;
}

void
compose_board_id(board_id_t & board_id)
{
	namespace F = stm32f42::flash;

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

	board_id.contents.version_hw = HELICO_VERSION_HW;
}

bool
format_board_id(const board_id_t & board_id, char (&s)[33])
{
	static_assert(sizeof board_id.items_u32 == 16,
			"Invalid board_id size assumption.");
	snprintf(s, sizeof s, "%08x%08x%08x%08x"
		, board_id.items_u32[0]
		, board_id.items_u32[1]
		, board_id.items_u32[2]
		, board_id.items_u32[3]
	);
}

bool
check_board_id(const char s[])
{
	board_id_t board_id;
	compose_board_id(board_id);

	char id_str[33];
	format_board_id(board_id, id_str);

	bool ok = strncasecmp(id_str, s, sizeof id_str) == 0;
	if (not ok) { fprintf(stderr, "Invalid board-id.\n"); }
	return ok;
}

bool
parse_stamp_content(const char * const arg[], FactoryStamp & stamp)
{
	static_assert(FactoryStamp::VERSION_TAG_CURRENT == 1,
		"parse_stamp_content() does not support new version tag.");

	uint32_t key_id;

	constexpr size_t sign_len = sizeof stamp.sign;
	uint8_t * const sign_bytes = (uint8_t*)stamp.sign;

	bool ok = parse_uint(arg[0], key_id);
	if (not ok) { fprintf(stderr, "Invalid key-id: %s\n", arg[0]); }

	if (ok)
	{
		ok = strlen(arg[1]) == 2 * sign_len
		     and decode_hex(arg[1], sign_bytes, sign_len) == sign_len;

		if (not ok)
			fprintf(stderr, "Invalid sign-128-hex: %s\n", arg[1]);
	}

	if (ok)
	{
		dbg("key-id: %u 0x%08x\n", key_id, key_id);

		memset(stamp.header.raw, 0xff, sizeof stamp.header.raw);

		stamp.magic = FactoryStamp::MAGIC;
		stamp.version_tag = 1;
		stamp.header.v1.version_hw = HELICO_VERSION_HW;
		stamp.header.v1.sign_key_id = key_id;

		auto p = (const uint8_t *)&stamp;
		dump_mem(stderr, p + 4, sizeof stamp - 4);
		stamp.crc32 = crc32(p + 4, sizeof stamp - 4);
	}
	return ok;
}

bool
write_otp_stamp(const FactoryStamp & stamp, bool burn)
{
	namespace F = stm32f42::flash;

	constexpr size_t START_BLOCK = 0;
	static_assert(START_BLOCK == 0,
		"AirDog::HardwareInfo requires stamp to be at the very first block."
		// Use for debug only with already spotted boards.
	);

	constexpr size_t SHIFT = START_BLOCK * 32;
	uint32_t * const stamp_u32 = (uint32_t*)&stamp;

	bool ok = stamp.magic == FactoryStamp::MAGIC
		and stamp.version_tag == FactoryStamp::VERSION_TAG_CURRENT;
	if (not ok) { fprintf(stderr, "Invalid version tag or magic.\n"); }

	if (ok)
	{
		ok = F::otp_is_untouched(SHIFT, sizeof stamp)
			or F::otp_verify_u32(SHIFT, stamp_u32, sizeof stamp / 4);
		if (not ok)
			fprintf(stderr, "OTP is spotted! Refusing to write.\n");
	}

	if (ok)
	{
		ok = not F::otp_any_byte_has_lock(SHIFT, sizeof stamp);
		if (not ok) { fprintf(stderr, "OTP is LOCKED!\n"); }
	}

	if (ok)
	{
		// start fLash programming mode
		F::write_guard guard;
		ok = F::status_ok() and
			F::otp_write_u32(guard, SHIFT, stamp_u32, sizeof stamp / 4);
		if (not ok) { fprintf(stderr, "OTP write failed!\n"); }
		// stop fLash programming mode
	}

	if (ok)
	{
		ok = F::otp_verify_u32(SHIFT, stamp_u32, sizeof stamp / 4);
		if (not ok) { fprintf(stderr, "Write verification FAILED!\n"); }
	}

	if (ok and burn)
	{
		// start fLash programming mode
		F::write_guard guard;
		ok = F::status_ok() and
			F::otp_burn_locks(guard, SHIFT, sizeof stamp);
		if (not ok) { fprintf(stderr, "OTP burn failed!\n"); }
		// stop fLash programming mode
	}

	return ok;
}

void
usage(const char name[], const char stamp_command[])
{
	fprintf(stderr
		, "Usage:\n"
		  "\t%s show-board-id\n"
		  "\t%s otp-stamp-info\n"
		  "\t%s [burn] %s board-id key-id sign-128-hex\n"
		  "\nWArning: %s destroys OTP even *without* burn!\n"
		, name
		, name
		, name, stamp_command
		, stamp_command
	);
}

}
// end of anonymous namespace

int
main(const int argc, const char * const argv[])
{
	char stamp_command[16];
	snprintf(stamp_command, sizeof stamp_command, "otp-stamp-v%u",
			FactoryStamp::VERSION_TAG_CURRENT);

	bool ok, usage_ok;
	if ((argc == 5 and streq(argv[1], stamp_command))
	or (argc == 6 and streq(argv[2], stamp_command))
	) {
		FactoryStamp stamp;

		const unsigned burn = argc == 6 and streq(argv[1], "burn");

		usage_ok = argc == 6 or not burn;
		ok = usage_ok
			and check_board_id(argv[2 + burn])
			and parse_stamp_content(argv + 3 + burn, stamp)
			and write_otp_stamp(stamp, burn);
	}
	else if (argc == 2 and streq(argv[1], "otp-stamp-info"))
	{
		usage_ok = ok = true;

		namespace F = stm32f42::flash;
		bool any = F::otp_any_byte_has_lock(0, sizeof(FactoryStamp));
		bool all = F::otp_all_bytes_are_locked(0, sizeof(FactoryStamp));
		printf("OTP stamp has lock: %s.\n", any ? "yes" : "no");
		printf("OTP stamp is locked: %s.\n", all ? "yes" : "no");
		printf("Version OTP: %u.\n", version_otp());
		printf("Version HW: %u.\n", version_hw());
		printf("Signature KeyID: %u.\n", signature_key_id());
	}
	else if (argc == 2 and streq(argv[1], "show-board-id"))
	{
		usage_ok = ok = true;

		board_id_t board_id;
		compose_board_id(board_id);

		char id_str[33];
		format_board_id(board_id, id_str);

		printf("%s\n", id_str);
	}
	else { usage_ok = ok = false; }


	if (not usage_ok)
	{
		usage(argv[0], stamp_command);
		return 1;
	}

	return ok ? 0 : 1;
}
