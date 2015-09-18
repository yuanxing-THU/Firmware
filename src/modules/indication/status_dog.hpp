#include <nuttx/config.h>

#ifndef CONFIG_ARCH_CHIP_STM32
# error Only STM32 supported.
#endif

#include <drivers/drv_hrt.h>
#include <board_leds.h>
#include <pthread.h>

#include <uORB/uORB.h>
#include <uORB/topics/bt_state.h>
#include <uORB/topics/vehicle_status.h>

#include "indication.h"

static pthread_mutex_t mutex_led_action = PTHREAD_MUTEX_INITIALIZER;

static led_action_t *led_actions = nullptr;
static unsigned int led_actions_size = 0;
static int led_action_repeat_count = 0;
static int led_pattern = -1;


static int vehicle_status_sub;
static int bt_state_sub;

static GLOBAL_BT_STATE bt_global_state = INITIALIZING;
static bool gps_valid = false;

#define LED_ON_INTENSITY 30

#define LED_PATTERN_SIZE(x) (sizeof(x) / sizeof(led_action_t))

#define LED_ACTION_ALL ( \
    LED_ACTION(LED_FL) | LED_ACTION(LED_FR) | \
    LED_ACTION(LED_RL) | LED_ACTION(LED_RR) \
)

static led_action_t led_pattern_on[] =
{
    {LED_ACTION_ALL, 1, LED_ON_INTENSITY, 0},
};

static led_action_t led_pattern_off[] =
{
    {LED_ACTION_ALL, 1, 0, 0},
};

static led_action_t led_pattern_blink_all[] =
{
    {LED_ACTION_ALL, 1, LED_ON_INTENSITY, 10},
    {LED_ACTION_ALL, 1, 0, 10},
};

static led_action_t led_pattern_blink_left_right[] =
{
    {LED_ACTION(LED_FL) | LED_ACTION(LED_RL), 1, LED_ON_INTENSITY, 100},
    {LED_ACTION_ALL, 1, 0, 50},
    {LED_ACTION(LED_FR) | LED_ACTION(LED_RR), 1, LED_ON_INTENSITY, 100},
    {LED_ACTION_ALL, 1, 0, 50},
};

static led_action_t led_pattern_dimm_slowly_all[] =
{
    {LED_ACTION_ALL, 10, 0, 200},
    {LED_ACTION_ALL, 10, LED_ON_INTENSITY, 200},
};

static led_action_t led_pattern_circle[] =
{
    {LED_ACTION_ALL, 1, 0, 0},
    {LED_ACTION(LED_FL), 1, LED_ON_INTENSITY, 100},
    {LED_ACTION(LED_FL), 1, 0, 50},
    {LED_ACTION(LED_FR), 1, LED_ON_INTENSITY, 100},
    {LED_ACTION(LED_FR), 1, 0, 50},
    {LED_ACTION(LED_RR), 1, LED_ON_INTENSITY, 100},
    {LED_ACTION(LED_RR), 1, 0, 50},
    {LED_ACTION(LED_RL), 1, LED_ON_INTENSITY, 100},
    {LED_ACTION(LED_RL), 1, 0, 50},
};

static led_action_t led_pattern_double_blink[] =
{
    {LED_ACTION_ALL, 1, 0, 0},
    {LED_ACTION_ALL, 1, LED_ON_INTENSITY, 100},
    {LED_ACTION_ALL, 1, 0, 150},
    {LED_ACTION_ALL, 1, LED_ON_INTENSITY, 100},
    {LED_ACTION_ALL, 1, 0, 650},
};

static led_action_t *led_patters[] = {
    led_pattern_on,
    led_pattern_off,
    led_pattern_blink_all,
    led_pattern_blink_left_right,
    led_pattern_dimm_slowly_all,
    led_pattern_circle,
    led_pattern_double_blink
};

static unsigned int led_patters_size[] = {
    LED_PATTERN_SIZE(led_pattern_on),
    LED_PATTERN_SIZE(led_pattern_off),
    LED_PATTERN_SIZE(led_pattern_blink_all),
    LED_PATTERN_SIZE(led_pattern_blink_left_right),
    LED_PATTERN_SIZE(led_pattern_dimm_slowly_all),
    LED_PATTERN_SIZE(led_pattern_circle),
    LED_PATTERN_SIZE(led_pattern_double_blink)
};

int indication_led_action(int pattern, int repeat_count)
{
    int result = 0;

    if (pattern >= LED_PATTERN_MAX)
    {
        result = -1;
    }

    if (result == 0)
    {
        pthread_mutex_lock(&mutex_led_action);

        led_pattern = pattern;
        led_actions = led_patters[pattern];
        led_actions_size = led_patters_size[pattern];
        led_action_repeat_count = repeat_count;

        pthread_mutex_unlock(&mutex_led_action);
    }

    return result;
}

namespace indication { namespace status {

void
init()
{
    led_perform_actions(led_pattern_on, LED_PATTERN_SIZE(led_pattern_on));
    sleep(5);
    led_perform_actions(led_pattern_off, LED_PATTERN_SIZE(led_pattern_off));

    vehicle_status_sub = orb_subscribe(ORB_ID(vehicle_status));
    bt_state_sub = orb_subscribe(ORB_ID(bt_state));

    // get current data
    bt_state_s bt_state;
    orb_copy(ORB_ID(bt_state), bt_state_sub, &bt_state);
    bt_global_state = bt_state.global_state;
}

void
update(hrt_abstime now)
{
    static bool isReady = false;

    bool actionPerformed = false;
    (void)now;

    pthread_mutex_lock(&mutex_led_action);

    if (led_action_repeat_count != 0)
    {
        led_perform_actions(led_actions, led_actions_size);
        actionPerformed = true;

        if (led_action_repeat_count > 0)
        {
            led_action_repeat_count--;
        }
    }

    pthread_mutex_unlock(&mutex_led_action);

    if (!actionPerformed)
    {
        bool updated = false;
        bt_state_s bt_state;

        orb_check(bt_state_sub, &updated);
        if (updated) {
            orb_copy(ORB_ID(bt_state), bt_state_sub, &bt_state);
            bt_global_state = bt_state.global_state;
        }

        if (bt_global_state == NO_PAIRED_DEVICES)
        {
            led_perform_actions(led_pattern_double_blink, LED_PATTERN_SIZE(led_pattern_double_blink));
            actionPerformed = true;
        }
        else if (bt_global_state == PAIRING)
        {
            led_perform_actions(led_pattern_blink_all, LED_PATTERN_SIZE(led_pattern_blink_all));
            actionPerformed = true;
        }
        else if (bt_global_state != CONNECTED)
        {
            led_perform_actions(led_pattern_circle, LED_PATTERN_SIZE(led_pattern_circle));
            actionPerformed = true;
        }
    }

    if (!actionPerformed)
    {
        // check gps
        bool updated = false;
        vehicle_status_s status;

        orb_check(vehicle_status_sub, &updated);
        if (updated) {
            orb_copy(ORB_ID(vehicle_status), vehicle_status_sub, &status);

            gps_valid = status.condition_global_position_valid &&
                    status.condition_home_position_valid &&
                    status.condition_target_position_valid;
        }

        if (!gps_valid)
        {
            led_perform_actions(led_pattern_circle, LED_PATTERN_SIZE(led_pattern_circle));
            actionPerformed = true;
        }
    }

    if (!actionPerformed)
    {
        if (isReady)
        {
            // turn leds on if not action was performed
           led_perform_actions(led_pattern_on, LED_PATTERN_SIZE(led_pattern_on));
        }
        else
        {
            isReady = gps_valid && bt_global_state == CONNECTED;
            if (isReady)
            {
                // dron is ready for action for the first time
                indication_led_action(LED_PATTERN_DIMM_SLOWLY_ALL, 3);
            }
        }
    }
}

void
done()
{}


}} // end of namespace indication::leds
