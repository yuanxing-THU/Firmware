#include <nuttx/config.h>

#include <poll.h>
#include <stm32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <board_config.h>
#include <dog_debug.hpp>
#include <systemlib/systemlib.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/uORB.h>

static bool g_thread_should_run = false;
static bool g_thread_running = false;

constexpr uint32_t cycle_time = 1000; // ms
constexpr uint32_t poll_timeout = 100; // ms
constexpr float init_epsilon = 0.0001; // how much above 0 battery needs to be to init
// TODO! [AK] Make this a parameter or an argument
constexpr float critical_level = 0.35f; // at which level kill the power

int thread_main(int argc, char ** argv) {
	int status_sub = orb_subscribe(ORB_ID(vehicle_status));
	orb_set_interval(status_sub, cycle_time);
	vehicle_status_s vehicle_status;
	pollfd poller;
	poller.fd = status_sub;
	poller.events = POLLIN;
	int res;
	bool inited = false;

	g_thread_running = true;
	while (g_thread_should_run) {
		res = poll(&poller, 1, cycle_time + poll_timeout);
		if (res == 1) {
			orb_copy(ORB_ID(vehicle_status), status_sub, &vehicle_status);
			DOG_PRINT("Current battery: %5.4f\n", (double) vehicle_status.battery_remaining);
			if (!inited) { // Wait for first correct estimation
				if (vehicle_status.battery_remaining >= (0.0f + init_epsilon)) {
					inited = true;
				}
			}
			else if (vehicle_status.battery_remaining < critical_level) {
				printf("Battery is low %5.4f shutting down power!\n", (double) vehicle_status.battery_remaining);
				stm32_gpiowrite(GPIO_VDD_FORCE_POWER, 0);
				stm32_gpiowrite(GPIO_VDD_3V3_SENSORS_EN, 0);
				stm32_gpiowrite(GPIO_VDD_PERIPHERY_EN, 0);
				stm32_gpiowrite(GPIO_VDD_ESC_EN, 0);
				stm32_gpiowrite(GPIO_VDD_RANGEFINDER_EN, 0);
				stm32_gpiowrite(GPIO_SENS_HEAT_EN, 0);
				// TODO! [AK] Ask BGC to disable the gimbal
				g_thread_should_run = false; // Our job here is done
				break;
			}
		}
		else {
			DOG_PRINT("poweroffer: poll error!\n");
			// TODO! [AK] Proper handle poll errors
		}
	}
	orb_unsubscribe(status_sub);
	g_thread_running = false;
}

extern "C" __EXPORT int poweroffer_main(int argc, char ** argv) {

	if (argc == 2 && strcmp(argv[1], "start") == 0) {
		if (!g_thread_running) {
			g_thread_should_run = true;
			task_spawn_cmd("poweroffer_daemon",
				SCHED_DEFAULT,SCHED_PRIORITY_DEFAULT, 2000,
				thread_main,
				(const char**) NULL);
		}
	}
	else if (argc == 2 && strcmp(argv[1], "stop") == 0) {
		g_thread_should_run = false;
		usleep(cycle_time + poll_timeout);
		if (g_thread_running) {
			usleep(cycle_time + poll_timeout);
		}
		if (!g_thread_running) {
			printf("Stopped.\n");
		}
		else {
			printf("Failed!\n");
		}
	}
	else if (argc == 2 && strcmp(argv[1], "status") == 0) {
		if (g_thread_running) {
			printf("Running\n");
		}
		else {
			printf("Not running\n");
		}
	}
	else {
		printf("Usage: start, stop or status. Nothing else.\n");
	}

	return 0;
}
