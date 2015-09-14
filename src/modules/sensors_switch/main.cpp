extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>

#include <board_config.h>
#include <drivers/drv_adc.h>
#include <uORB/uORB.h>
#include <uORB/topics/system_power.h>

#define SENSOR_SWITCH_GPIO GPIO_VDD_3V3_SENSORS_EN

#ifndef N_ADC_CHANNELS
// same as in sensors.cpp
# define N_ADC_CHANNELS 12
#endif

namespace {

inline bool
streq(const char *a, const char *b) { return not std::strcmp(a, b); }

bool
report_system_voltage()
{
	int sub = orb_subscribe(ORB_ID(system_power));
	if (sub == -1)
	{
		perror("orb_subscribe(system_power)");
		return false;
	}

	system_power_s system_power;
	orb_copy(ORB_ID(system_power), sub, &system_power);
	orb_unsubscribe(sub);

	printf("System voltage: %.2f\n", (double)system_power.voltage5V_v);
	printf("Servo valid: %d, brick valid: %d, usb conncted: %d.\n"
		, system_power.servo_valid
		, system_power.brick_valid
		, system_power.usb_connected);

	return true;
}

bool
read_adc(struct adc_msg_s (&adc)[N_ADC_CHANNELS])
{
	int f = open(ADC_DEVICE_PATH, O_RDONLY);

	if (f == -1)
	{
		perror(ADC_DEVICE_PATH);
		return false;
	}

	ssize_t r = read(f, &adc, sizeof adc);
	if (r != sizeof adc) { perror(ADC_DEVICE_PATH); }

	close(f);

	return r == sizeof adc;
}

bool
report_sensor_voltage()
{
	struct adc_msg_s adc[N_ADC_CHANNELS];
	if (not read_adc(adc)) { return false; }

	bool ok = false;
	for (size_t i = 0; i < N_ADC_CHANNELS; ++i)
	{
		fprintf(stderr, "%u adc %i data %4u (0x%04x)\n",
				i, adc[i].am_channel,
				adc[i].am_data, adc[i].am_data);
#ifdef ADC_SENSORS_VOLTAGE_CHANNEL
		if (adc[i].am_channel == ADC_SENSORS_VOLTAGE_CHANNEL)
		{
			printf("Sensor voltage: %u %.2f\n"
				, adc[i].am_data
				, adc[i].am_data * ADC_SENSORS_VOLTAGE_SCALE
			);
			ok = true;
		}
#endif
	}
	if (not ok)
		fprintf(stderr, "Sensor voltage undetermined.\n");
	return ok;
}

bool
report_channel(unsigned channel, float scale)
{
	struct adc_msg_s adc[N_ADC_CHANNELS];
	if (not read_adc(adc)) { return false; }

	for (size_t i = 0; i < N_ADC_CHANNELS; ++i)
		if (adc[i].am_channel == channel)
		{
			printf("channel %u raw 0x%03x value %.2f\n"
				, channel
				, adc[i].am_data
				, double(adc[i].am_data * scale)
			);
			return true;
		}

	return false;
}

bool
factory_adc_check()
{
#if CONFIG_ARCH_BOARD_AIRLEASH

	return report_channel(
			ADC_SYSPOWER_VOLTAGE_CHANNEL,
			ADC_SYSPOWER_VOLTAGE_SCALE
	);

#elif CONFIG_ARCH_BOARD_AIRDOG_FMU
    return false;
#elif CONFIG_ARCH_BOARD_PX4FMU_V2
    return false;
#else
/*
 * If you need to compile the stuff on some other board,
 * add explicit
 *
 *     return false
 *
 * for the case.
 */
# error Unsupported board.
#endif
}

} // end of namespace

int
main(int argc, const char * const * const argv)
{
	if (argc < 2)
	{
		fprintf(stderr,
			"Usage: %s command [...]\n"
			"\n"
			"Command is one of: on, off, status, 3ms, factory-adc-check."
			" Each could be repeated multiple times\n"
			, argv[0]);
		return 1;
	}

	bool ok = false;
	for (int i = 1; i < argc; ++i)
	{
		if (streq(argv[i], "on"))
		{
			ok = true;
			printf("%s\n", argv[i]);
			stm32_gpiowrite(SENSOR_SWITCH_GPIO, 1);
		}
		else if (streq(argv[i], "off"))
		{
			ok = true;
			printf("%s\n", argv[i]);
			stm32_gpiowrite(SENSOR_SWITCH_GPIO, 0);
		}
		else if (streq(argv[i], "status"))
		{
			bool on = stm32_gpioread(SENSOR_SWITCH_GPIO);
			printf("Sensor power %s\n", on ? "on" : "off");
			ok = report_sensor_voltage();
			ok = report_system_voltage() and ok;
		}
		else if (streq(argv[i], "3ms"))
		{
			ok = true;
			printf("%s\n", argv[i]);
			usleep(3000);
		}
		else if (streq(argv[i], "factory-adc-check"))
		{
			ok = factory_adc_check();
		}
		else { fprintf(stderr, "Unknown command: \"%s\"\n", argv[i]); }
	}

	return ok ? 0 : 1;
}
