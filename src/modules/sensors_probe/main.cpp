extern "C" __EXPORT int main(int argc, const char * const * argv);

#include <nuttx/config.h>
#include <nuttx/spi.h>

#include <cstring>
#include <cstdio>

#include <board_config.h>

namespace {

using sens_name_t = const char * const;
struct sens_HMC5883 { static constexpr sens_name_t name = "hmc5883"; };
struct sens_L3GD20 { static constexpr sens_name_t name = "l3gd20"; };
struct sens_LSM303D { static constexpr sens_name_t name = "lsm303d"; };
struct sens_MPU6000 { static constexpr sens_name_t name = "mpu6000"; };
struct sens_MS5611 { static constexpr sens_name_t name = "ms5611"; };

constexpr uint8_t READ(uint8_t r) { return r | 0x80; }
constexpr uint8_t AUTOINC(uint8_t r) { return r | 0x40; }

struct spi_dev_s *
init(int bus) {
	auto dev = up_spiinitialize(bus);
	SPI_SETFREQUENCY(dev, 1000000);
	//SPI_SETMODE(dev, _mode);
	//SPI_SETBITS(dev, 8);
	return dev;
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

void
exchange(spi_dev_s * dev, int dev_no, uint8_t send[], uint8_t recv[], size_t len)
{
	SPI_LOCK(dev, true);
	SPI_SELECT(dev, (spi_dev_e)dev_no, true);

	printf("dev #%i send %u: ", dev_no, len);
	print_hex(send, len);
	printf("\n");

	SPI_EXCHANGE(dev, send, recv, len);

	printf("dev #%i recv %u: ", dev_no, len);
	print_hex(recv, len);
	printf("\n");

	SPI_SELECT(dev, (spi_dev_e)dev_no, false);
	SPI_LOCK(dev, false);
}

inline uint8_t
read_reg(spi_dev_s * dev, int dev_no, uint8_t reg)
{
	uint8_t buf[2] = { READ(reg), 0 };
	exchange(dev, dev_no, buf, buf, sizeof(buf));
	return buf[1];
}

bool
probe(sens_L3GD20, spi_dev_s * dev)
{ return read_reg(dev, PX4_SPIDEV_GYRO, 0x0f) == 0xd4; }

bool
probe(sens_LSM303D, spi_dev_s * dev)
{ return read_reg(dev, PX4_SPIDEV_ACCEL_MAG, 0x0f) == 0x49; }

bool
probe(sens_MPU6000, spi_dev_s * dev)
{
	uint8_t revisions[] = { 0x14, 0x15, 0x16, 0x17, 0x18, 0x54, 0x55, 0x56,
				0x57, 0x58, 0x59, 0x5A };
	uint8_t id = read_reg(dev, PX4_SPIDEV_MPU, 0x0c);
	return memchr(revisions, id, sizeof(revisions)) != nullptr;
}

bool
probe(sens_MS5611, spi_dev_s * dev)
{
	uint8_t buf[3] = {0, 0, 0};

	buf[0] = 0x1e;
	exchange(dev, PX4_SPIDEV_BARO, buf, buf, 1);

	bool ok;
	uint8_t i = 0;
	do
	{
		buf[0] = 0xa0 | i;
		buf[1] = 0;
		buf[2] = 0;

		exchange(dev, PX4_SPIDEV_BARO, buf, buf, 3);

		ok = (buf[1] != buf[2]) or ((buf[1] != 0) and (buf[1] != 0xff));

		i += 2;
	}
	while(not ok and i < 16);

	return ok;
}

uint8_t
probe(sens_HMC5883, spi_dev_s * dev)
{
#ifdef SPI_HMC5883_DEV
	uint8_t buf[4] = { READ(AUTOINC(0x0a)), 0, 0, 0 };
	exchange(dev, SPI_HMC5883_DEV, buf, buf, sizeof(buf));
	return buf[1] == 'H' and buf[2] == '4' and buf[3] == '3';
#else
#endif
}

template <typename S>
inline bool
probe_report(spi_dev_s * dev, S s)
{
	bool ok = probe(s, dev);
	printf("%s: %s\n", S::name, ok ? "ok" : "fail");
	return ok;
}

bool
streq(const char a[], const char b[]) { return strcmp(a, b) == 0; }

void
usage(const char name[])
{
	fprintf(stderr
		, "Usage: %s [sensor ...]\n"
		  "\n"
		  "Where sensor is one of:\n"
		  "\t%s\n"
		  "\t%s\n"
		  "\t%s\n"
		  "\t%s\n"
#ifdef SPI_HMC5883_DEV
		  "\t%s\n"
#endif
		  "No arguments mean all sensors.\n"
		  "\n"
		, name
		, sens_L3GD20::name
		, sens_LSM303D::name
		, sens_MPU6000::name
		, sens_MS5611::name
#ifdef SPI_HMC5883_DEV
		, sens_HMC5883::name
#endif
	);
}

} // end of namespace

int
main(int argc, const char * const * argv)
{
	struct spi_dev_s * dev = init(PX4_SPI_BUS_SENSORS);
	if (dev == nullptr)
	{
		fprintf(stderr, "SPI bus initialization error.\n");
		return 1;
	}

	bool ok = true;

	if (argc == 1)
	{
		ok = probe_report(dev, sens_L3GD20()) and ok;
		ok = probe_report(dev, sens_LSM303D()) and ok;
		ok = probe_report(dev, sens_MPU6000()) and ok;
		ok = probe_report(dev, sens_MS5611()) and ok;
#ifdef SPI_HMC5883_DEV
		ok = probe_report(dev, sens_HMC5883()) and ok;
#endif
	}
	else
	{
		for (int i = 1; i < argc; ++i)
		{
			if (streq(argv[i], sens_L3GD20::name))
				ok = probe_report(dev, sens_L3GD20()) and ok;
			else if (streq(argv[i], sens_LSM303D::name))
				ok = probe_report(dev, sens_LSM303D()) and ok;
			else if (streq(argv[i], sens_MPU6000::name))
				ok = probe_report(dev, sens_MPU6000()) and ok;
			else if (streq(argv[i], sens_MS5611::name))
				ok = probe_report(dev, sens_MS5611()) and ok;
#ifdef SPI_HMC5883_DEV
			else if (streq(argv[i], sens_HMC5883::name))
				ok = probe_report(dev, sens_HMC5883()) and ok;
#endif
			else
			{
				usage(argv[0]);
				return 1;
			}
		}
	}
	return ok ? 0 : 1;
}
