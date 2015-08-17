extern "C" __EXPORT int main(int argc, const char * const * argv);

#include <nuttx/config.h>
#include <nuttx/i2c.h>

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#define MAX_LEN 128

namespace {

struct i2c_dev_s *
init(int bus_no) { return up_i2cinitialize(bus_no); }

void
transfer(i2c_dev_s * dev, uint16_t addr, uint16_t flags, uint8_t buf[], size_t len)
{
	struct i2c_msg_s msg = { addr, flags, buf, int(len) };
	I2C_SETFREQUENCY(dev, 100000);
	auto ret = I2C_TRANSFER(dev, &msg, 1);
	if (ret == 0) { printf("OK\n"); }
	else { fprintf(stderr, "I2C_TRANSFER: %i: %s\n", -ret, strerror(-ret)); }
}

bool
parse_uint(const char s[], unsigned &n)
{
	char *p;
	n = std::strtoul(s, &p, 0);
	return *p == '\0';
}

uint8_t
decode_hexdigit(uint8_t x)
{
	if (x < '0')
		return 0;
	if (x <= '9')
		return x - uint8_t('0');
	if (x < 'A')
		return 0;
	if (x <= 'F')
		return x - uint8_t('A') + 10;
	if (x < 'a')
		return 0;
	if (x <= 'f')
		return x - uint8_t('a') + 10;
	return 0;
}

uint8_t
decode_hex_8bit(char h, char l)
{ return (decode_hexdigit(h) << 4) | decode_hexdigit(l); }

size_t
decode_hex(const char *s, uint8_t buf[], size_t max_len)
{
	size_t i = 0;
	while (i < max_len and *s)
	{
		char l, h;
		h = *s;
		++s;
		l = *s;
		++s;
		buf[i] = decode_hex_8bit(h, l);
		++i;
	}
	return i;
}

void
print_hex_8bit(uint8_t x) { printf(" %02x", x); }

void
print_hex(const uint8_t * buf, size_t len)
{
	while (len) {
		print_hex_8bit(*buf);
		++buf;
		--len;
	}
}

bool
streq(const char a[], const char b[]) { return strcmp(a, b) == 0; }

} // end of namespace

int
main(int argc, const char * const * argv)
{
	int i = 0;
	uint8_t buf[MAX_LEN];
	for (int j=0; j < MAX_LEN; ++j) { buf[j] = uint8_t(MAX_LEN - j); }

	if (argc < 4)
	{
		fprintf(stderr, "Usage:\n"
				"\t%s BUS ADDR write hex-string\n"
				"\t%s BUS ADDR read len\n"
				, argv[0], argv[0]);
		return 1;
	}
	++i;
	++argv;

	unsigned bus_no;
	if (not parse_uint(*argv, bus_no))
	{
		fprintf(stderr, "Invalid bus number: %s\n", *argv);
		return 1;
	}
	++i;
	++argv;

	auto * dev = init(bus_no);
	if (dev == nullptr)
	{
		fprintf(stderr, "Failed to initialize I2C BUS %u\n", bus_no);
		return 1;
	}

	unsigned addr;
	if (not parse_uint(*argv, addr) or addr > 0xFFFF)
	{
		fprintf(stderr, "Invalid address: %s\n", *argv);
		return 1;
	}
	++i;
	++argv;

	if (streq(*argv, "write"))
	{
		++i;
		++argv;

		size_t len = 0;
		for(; i < argc; ++i, ++argv)
			len += decode_hex(*argv, buf + len, sizeof buf - len);

		printf("sending: ");
		print_hex(buf, len);
		printf("\n");

		transfer(dev, uint16_t(addr), 0, buf, len);
	}
	else if (streq(*argv, "read"))
	{
		++i;
		++argv;

		size_t len;
		if (not parse_uint(*argv, len))
		{
			fprintf(stderr, "Invalid length: %s\n", *argv);
			return 1;
		}
		else if (len >= sizeof buf)
		{
			fprintf(stderr, "Length limited by: %u\n", sizeof buf);
			len = sizeof buf;
		}

		transfer(dev, uint16_t(addr), I2C_M_READ, buf, len);

		printf("received: ");
		print_hex(buf, len);
		printf("\n");
	}

	up_i2cuninitialize(dev);

	return 0;
}
