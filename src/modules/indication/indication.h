#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// API supported only in airdog
#ifdef CONFIG_ARCH_BOARD_AIRDOG_FMU

enum
{
    LED_PATTERN_ON,
    LED_PATTERN_OFF,
    LED_PATTERN_BLINK_ALL,
    LED_PATTERN_BLINK_LEFT_RIGHT,
    LED_PATTERN_DIMM_SLOWLY_ALL,
    LED_PATTERN_CIRCLE,
    LED_PATTERN_DOUBLE_BLINK,
    LED_PATTERN_MAX
};

int indication_led_action(int pattern, int repeat_count);


#else
int indication_led_action(int pattern, int repeat_count)
{
    // do nothing
    (void)pattern;
    (void)repeat_count;
}
#endif

#ifdef __cplusplus
}
#endif
