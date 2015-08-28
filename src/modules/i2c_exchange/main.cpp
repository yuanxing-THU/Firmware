extern "C" __EXPORT int main(int argc, const char * const * argv);

#include <nuttx/config.h>
#include <nuttx/i2c.h>

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#define dbg(...) DOG_PRINT(__VA_ARGS__)

#define BUF_LEN 256
#define N_BUFS 4

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

void
usage(const char name[])
{
	fprintf(stderr
		, "Usage:\n"
		  "\t%s BUS ADDR read len\n"
		  "\t%s BUS ADDR write hex-string\n"
		  "\t%s BUS ADDR write hex-string read len\n"
		, name, name, name
	);
}

} // end of namespace

int
main(int argc, const char * const * argv)
{
	const char * const name = *argv;
	int i = 0;

	if (argc < 5)
	{
		dbg("argc %i\n", argc);
		usage(name);
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

	unsigned address;
	if (not parse_uint(*argv, address) or address > 0xFFFF)
	{
		fprintf(stderr, "Invalid address: %s\n", *argv);
		return 1;
	}
	++i;
	++argv;

	size_t n_msg = 0;
	struct i2c_msg_s msg[N_BUFS];
	uint8_t buf[N_BUFS][BUF_LEN];

	for (size_t k = 0; k < N_BUFS; ++k)
		for (size_t j = 0; j < BUF_LEN; ++j)
			buf[k][j] = 0xff - j;

	for(; i < argc; ++i, ++argv)
	{
		if (streq(*argv, "write"))
		{
			if (n_msg == N_BUFS)
			{
				fprintf(stderr, "Too many commands.\n");
				return 1;
			}
			msg[n_msg].addr = address;
			msg[n_msg].flags = 0;
			msg[n_msg].buffer = &buf[n_msg][0];
			msg[n_msg].length = 0;
			++n_msg;
		}
		else if (streq(*argv, "read"))
		{
			if (n_msg == N_BUFS)
			{
				fprintf(stderr, "Too many commands.\n");
				return 1;
			}
			msg[n_msg].addr = address;
			msg[n_msg].flags = I2C_M_READ;
			msg[n_msg].buffer = reinterpret_cast<uint8_t *>(buf + n_msg);
			msg[n_msg].length = 0;
			++n_msg;
		}
		else if (n_msg == 0)
		{
			dbg("n_msg == 0\n");
			usage(name);
			return 1;
		}
		else if (msg[n_msg - 1].flags == I2C_M_READ)
		{
			size_t k = n_msg - 1;
			if (msg[k].length != 0)
			{
				fprintf(stderr, "Too many arguments for read.\n");
				return 1;
			}
			size_t len;
			if (not parse_uint(*argv, len) or len == 0)
			{
				fprintf(stderr, "Invalid length: %s\n", *argv);
				return 1;
			}
			else if (len >= sizeof buf)
			{
				fprintf(stderr, "Length limited by: %u\n", sizeof buf);
				len = sizeof buf;
			}

			msg[k].length = len;
		}
		else // if (msg[n_msg - 1].flags == 0)
		{
			size_t k = n_msg - 1;
			if (msg[k].length == BUF_LEN
			or strlen(*argv) > 2 * BUF_LEN - msg[n_msg].length
			) {
				fprintf(stderr, "Argument %u is out buffer.\n", i);
				return 1;
			}
			size_t len = decode_hex(
				*argv,
				msg[k].buffer + msg[k].length,
				BUF_LEN - msg[k].length
			);

			if (len == 0)
			{
				fprintf(stderr, "Invalid argument %u.\n", i);
				return 1;
			}

			dbg("decode_hex('%s', %u, %u) -> %u.\n"
				, *argv
				, msg[k].length
				, BUF_LEN - msg[k].length
				, len
			);

			// TODO if (len == 0) { }
			msg[k].length += len;
		}
		dbg("msg[%i].length %i.\n", n_msg - 1, msg[n_msg - 1].length);
	}

	printf("\n");
	for (size_t k = 0; k < n_msg; ++k)
	{
		if (msg[k].flags == I2C_M_READ)
			printf("Read %i.\n", msg[k].length);
		else
		{
			printf("Write %i:", msg[k].length);
			print_hex(msg[k].buffer, msg[k].length);
			printf(".\n");
		}
	}
	printf("\n");

	I2C_SETFREQUENCY(dev, 100000);
	auto ret = I2C_TRANSFER(dev, msg, n_msg);

	if (ret == 0) { printf("OK\n"); }
	else { fprintf(stderr, "I2C_TRANSFER: %i: %s\n", -ret, strerror(-ret)); }

	printf("\n");
	for (size_t k = 0; k < n_msg; ++k)
	{
		const char * const mode =
			msg[k].flags == I2C_M_READ ? "Read" : "Write";
		printf("%s %i:", mode, msg[k].length);
		print_hex(msg[k].buffer, msg[k].length);
		printf(".\n");
	}
	printf("\n");

	up_i2cuninitialize(dev);

	return 0;
}
