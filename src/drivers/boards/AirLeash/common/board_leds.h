#pragma once

__BEGIN_DECLS

enum {
    LED_RED,
    LED_BLUE,
    LED_SIZE
};

#define N_LEDS 2
#define LED_STATUS	0
#define LED_LEASHED	1

#define LED_TOGGLE_MAX_Hz	10

#define LED_GPIO_ACTIVE_LOW	1
#define GPIO_LED_RED    (GPIO_ALT|GPIO_AF1|GPIO_SPEED_50MHz|GPIO_PUSHPULL|GPIO_PORTE|GPIO_PIN12)
#define GPIO_LED_BLUE   (GPIO_ALT|GPIO_AF1|GPIO_SPEED_50MHz|GPIO_PUSHPULL|GPIO_PORTE|GPIO_PIN14)

__EXPORT void led_init();
__EXPORT int led_set_intensity(int led, float intensity);
__EXPORT int led_on(int led);
__EXPORT int led_off(int led);

__END_DECLS
