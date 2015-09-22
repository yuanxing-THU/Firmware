#include <nuttx/config.h>

#ifndef CONFIG_ARCH_CHIP_STM32
# error Only STM32 supported.
#endif

#include <cstdint>
#include <cstdio>
#include <ctime>
#include <fcntl.h> // open

#include <board_leds.h>
#include <drivers/drv_hrt.h>
#include <drivers/drv_tone_alarm.h>
#include <drivers/boards/AirLeash/common/board_leds.h>
#include <systemlib/param/param.h>
#include <modules/leash/kbd_defines.hpp>
#include <modules/commander/commander_error.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/leash_status.h>

#include "leds.hpp"

namespace indication { namespace status {

using kbd_handler::ModeId;

static int status_sub;
static int airdog_sub;
static int leash_sub;

static param_t param_use_blue_led;
static int32_t use_blue_led;

static bool link_valid, force_update;
leash_status_s l_status;

void
init()
{
	status_sub = orb_subscribe(ORB_ID(vehicle_status));
	airdog_sub = orb_subscribe(ORB_ID(airdog_status));
    leash_sub = orb_subscribe(ORB_ID(leash_status));
	param_use_blue_led = param_find("LEASH_USE_BLUE_L");
	force_update = true;
	link_valid = false;
	use_blue_led = -1;
}

void
update(hrt_abstime now)
{
    static int _error_code = -1;
	vehicle_status_s status;
	orb_copy(ORB_ID(vehicle_status), status_sub, &status);

    bool battery_error_updated = false;
	bool changed_link_valid = false;
    bool updated = false;
    airdog_status_s airdog_status_data;
    orb_check(airdog_sub, &updated);
    if (updated)
    {
        orb_copy(ORB_ID(airdog_status), airdog_sub, &airdog_status_data);
        if (_error_code != airdog_status_data.error_code)
        {
            _error_code = airdog_status_data.error_code;
            battery_error_updated = true;
        }
        bool link_ok = airdog_status_data.timestamp > 0
                and now - airdog_status_data.timestamp < 3000000;
        changed_link_valid = link_valid != link_ok;
        link_valid = link_ok;
    }


	bool led_updated = false;
	orb_check(leash_sub, &led_updated);
    if (led_updated) {
        orb_copy(ORB_ID(leash_status), leash_sub, &l_status);
    }





	int32_t x;
	param_get(param_use_blue_led, &x);
	bool changed_use_blue_led = x != use_blue_led;
	use_blue_led = x;

	if (force_update or
            changed_use_blue_led or
            changed_link_valid or
            led_updated or
            battery_error_updated)
    {
		uint32_t pattern;
        uint32_t tone_pattern;

		if (use_blue_led) {
			switch(_error_code) {
                case BSC_ERROR_BATTERY_RTH_NOTIFY:
                case BSC_ERROR_BATTERY_LAND_NOTIFY:
                    pattern = 0x1000;
                    tone_pattern = TONE_BATTERY_WARNING_SLOW_TUNE;
                    break;
                case BSC_ERROR_BATTERY_RTH:
                case BSC_ERROR_BATTERY_LAND:
                    pattern = 0x5000;
                    tone_pattern = TONE_BATTERY_WARNING_FAST_TUNE;
                    break;
                case BSC_ERROR_BATTERY_RTH_WO_BATT:
                case BSC_ERROR_BATTERY_LAND_DEATH:
                    pattern = 0xA;
                    tone_pattern = TONE_BATTERY_WARNING_FAST_TUNE;
                    break;
                default:
                    tone_pattern = TONE_STOP_TUNE;
                    pattern = 0x0;
                    break;
			}
            int beeper_fd = open(TONEALARM_DEVICE_PATH, O_RDONLY);
            led_set_intensity(LED_RED,100);
            ioctl(beeper_fd, TONE_SET_ALARM, tone_pattern);
			leds::set_pattern_repeat(LED_STATUS, pattern);

			if (link_valid)
				pattern = 0xFFFFFFFd;
//  0xFF00FF00
			else
				pattern = 0x80000000;

			leds::set_pattern_repeat(LED_LEASHED, pattern);
		}
		else
		{
			switch((ModeId) l_status.menu_mode) {
			case ModeId::FLIGHT:
				pattern = 0xFFFFFFFe;
				break;
			case ModeId::FLIGHT_ALT:
				pattern = 0xFFFFFFFa;
				break;
			case ModeId::FLIGHT_CAM:
				pattern = 0xFFFFFFea;
				break;
			default:
				pattern = 0x15000000;
				break;
			}
			leds::set_pattern_repeat(LED_STATUS, pattern);
			leds::set_pattern_repeat(LED_LEASHED, 0xFFFFFFFF);
		}
	}

	force_update = false;
}

void
done()
{ orb_unsubscribe(status_sub); }


}} // end of namespace indication::leds
