#pragma once

__BEGIN_DECLS

enum
{
    LED_FL,
    LED_FR,
    LED_RL,
    LED_RR,
    LED_SIZE
};

#define LED_TOGGLE_MAX_Hz	10

#define LED_ACTION(x) (1 << x)

#define GPIO_LED_FL     (GPIO_ALT|GPIO_AF1|GPIO_SPEED_50MHz|GPIO_PUSHPULL|GPIO_PORTE|GPIO_PIN14)
#define GPIO_LED_FR     (GPIO_ALT|GPIO_AF1|GPIO_SPEED_50MHz|GPIO_PUSHPULL|GPIO_PORTE|GPIO_PIN11)
#define GPIO_LED_RL     (GPIO_ALT|GPIO_AF1|GPIO_SPEED_50MHz|GPIO_PUSHPULL|GPIO_PORTE|GPIO_PIN13)
#define GPIO_LED_RR     (GPIO_ALT|GPIO_AF1|GPIO_SPEED_50MHz|GPIO_PUSHPULL|GPIO_PORTA|GPIO_PIN8)

typedef struct led_action_s
{
    unsigned char leds; // bit mask with led numbers
    unsigned char iteration; // how much iteration will be
    float intensity;
    unsigned int duration; // in milliseconds
} led_action_t;

__EXPORT void led_init(void);
__EXPORT int led_set_intensity(int led, float intensity);
__EXPORT int led_on(int led);
__EXPORT int led_off(int led);
__EXPORT int led_perform_actions(led_action_t *actions, unsigned int size);

__END_DECLS
