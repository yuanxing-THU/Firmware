#include <nuttx/config.h>

extern "C" __EXPORT
int main(int, const char * const []);

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <crc32.h>

#include <systemlib/otp.h>

#include <stm32f4/flash.hpp>

namespace
{

void
print_hex(FILE * f, uint8_t x) { fprintf(f, "%02x", unsigned(x)); }

void
print_hex(FILE * f, uint16_t x) { fprintf(f, "%04x", unsigned(x)); }

void
print_hex(FILE * f, uint32_t x) { fprintf(f, "%08x", unsigned(x)); }

template <typename U>
void
print_hex(FILE * f, const volatile U * addr, size_t n)
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
	return parse_uint(s, n, tail) and tail and *tail == '\0';
}

void
usage(const char name[])
{
	fprintf(stderr
		, "Usage:"
		  "\t%s dump 8|16|32 0xADDrEss count\n"
		  "\t%s dump 8|16|32 OTP|OTP-LOCK|UDID [count]\n"
		  "\t%s crc32 0xADDrEss count\n"
		  "\t%s crc32 OTP|OTP-LOCK|UDID [count]\n"
		, name, name, name, name
	);
}

bool
check_align(size_t addr, size_t align_bytes)
{
	bool ok = (addr & (align_bytes - 1)) == 0;
	if (not ok)
	{
		fprintf(stderr, "Required alignment at %u byte boundary.\n",
				align_bytes);
	}
	return ok;
}

bool
parse_address_size(const char a[], const char s[], size_t & addr, size_t & size)
{
	if (streq(a, "OTP"))
	{
		addr = ADDR_OTP_START;
		size = 512;
	}
	else if (streq(a, "OTP-LOCK"))
	{
		addr = ADDR_OTP_LOCK_START;
		size = 16;
	}
	else if (streq(a, "UDID"))
	{
		namespace F = stm32f42::flash;
		addr = reinterpret_cast<uint32_t>(F::mcu_udid);
		size = sizeof F::mcu_udid;
	}
	else if (parse_uint(a, addr))
	{
		size = 0;
	}
	else
	{
		return false;
	}

	if (s == nullptr) { return size != 0; }

	return parse_uint(s, size);
}

}
// end of anonymous namespace

int
main(int argc, const char * const argv[])
{
	if ((argc == 4 or argc == 5) and streq(argv[1], "dump"))
	{
		uint32_t bits;
		if (not parse_uint(argv[2], bits))
		{
			usage(argv[0]);
			return 1;
		}

		bool predefined = argc == 4;

		size_t addr, n;
		if (not parse_address_size(argv[3], argv[4], addr, n))
		{
			usage(argv[0]);
			return 1;
		}

		if (bits == 16)
		{
			if (not check_align(addr, 2)) { return 1; }
			if (predefined) { n /= 2; }
		}
		else if (bits == 32)
		{
			if (not check_align(addr, 4)) { return 1; }
			if (predefined) { n /= 4; }
		}

		printf("0x%08x: ", addr);

		auto p = reinterpret_cast<const volatile void *>(addr);
		switch (bits)
		{
		case 8:
			print_hex(stdout, (const volatile uint8_t *)p, n);
			break;
		case 16:
			print_hex(stdout, (const volatile uint16_t *)p, n);
			break;
		case 32:
			print_hex(stdout, (const volatile uint32_t *)p, n);
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}
	else if ((argc == 3 or argc == 4) and streq(argv[1], "crc32"))
	{
		size_t addr, n;
		if (not parse_address_size(argv[2], argv[3], addr, n))
		{
			usage(argv[0]);
			return 1;
		}

		auto p = reinterpret_cast<const uint8_t *>(addr);
		printf("0x%08x\n", crc32(p, n));

	}
	else { usage(argv[0]); }

	return 0;
}
