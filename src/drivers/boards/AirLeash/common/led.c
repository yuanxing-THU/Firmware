/****************************************************************************
 *
 *   Copyright (c) 2013 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file px4fmu2_led.c
 *
 * PX4FMU LED backend.
 */

#include <nuttx/config.h>

#include <stdbool.h>

#include <stm32.h>
#include <stm32_gpio.h>
#include <stm32_tim.h>
#include <arch/board/board.h>

#include "board_config.h"
#include "board_leds.h"

#define REG(_reg)   (*(volatile uint32_t *)(_reg))
#define TIMER_CR1_EN (1 << 0)

static struct {
    int active;
    float intensity;
} leds[LED_SIZE] = {
    { 0,  7 }, // red, look a little more bright than blue
    { 0,  9 }, // blue
};

/*
 * Ideally we'd be able to get these from up_internal.h,
 * but since we want to be able to disable the NuttX use
 * of leds for system indication at will and there is no
 * separate switch, we need to build independent of the
 * CONFIG_ARCH_LEDS configuration switch.
 */

void led_init()
{
    stm32_configgpio(GPIO_LED_RED);
    stm32_configgpio(GPIO_LED_BLUE);

    // enable timer
    modifyreg32(STM32_RCC_APB2ENR, 0, RCC_APB2ENR_TIM1EN);

    // disable timer and configure it
    REG(STM32_TIM1_CR1) = 0;
    REG(STM32_TIM1_CR2) = 0;
    REG(STM32_TIM1_SMCR) = 0;
    REG(STM32_TIM1_DIER) = 0;
    REG(STM32_TIM1_SR) = 0;
    REG(STM32_TIM1_EGR) = GTIM_EGR_UG;
    REG(STM32_TIM1_CCMR1) = 0;
    REG(STM32_TIM1_CCMR2) =
            (GTIM_CCMR_MODE_PWM1 << GTIM_CCMR2_OC4M_SHIFT) | GTIM_CCMR2_OC4PE |
            (GTIM_CCMR_MODE_PWM1 << GTIM_CCMR2_OC3M_SHIFT) | GTIM_CCMR2_OC3PE;
    REG(STM32_TIM1_CCER) = GTIM_CCER_CC3NE | GTIM_CCER_CC4E;
    REG(STM32_TIM1_CNT) = 0;
    REG(STM32_TIM1_PSC) = 0;
    REG(STM32_TIM1_ARR) = 0;
    REG(STM32_TIM1_RCR) = 0;
    REG(STM32_TIM1_CCR1) = 0;
    REG(STM32_TIM1_CCR2) = 0;
    REG(STM32_TIM1_CCR3) = UINT16_MAX;
    REG(STM32_TIM1_CCR4) = UINT16_MAX;
    REG(STM32_TIM1_BDTR) = ATIM_BDTR_MOE;
    REG(STM32_TIM1_DCR) = 0;
    REG(STM32_TIM1_DMAR) = 0;

    REG(STM32_TIM1_PSC) = 10;

    // run the full span of the counter. All timers can handle uint16
    REG(STM32_TIM1_ARR) = UINT16_MAX;

    // generate an update event; reloads the counter, all registers
    REG(STM32_TIM1_EGR) = GTIM_EGR_UG | GTIM_EGR_CC4G | GTIM_EGR_CC3G;

    // enable the timer
    REG(STM32_TIM1_CR1) = GTIM_CR1_CEN;

    led_on(LED_BLUE);
    led_on(LED_RED);
}

static int led_set_ccr(int led, int ccr)
{
    int result = 0;

    if (led >= LED_SIZE || led < 0) {
        result = -1;
    }

    if (result == 0)
    {
        switch (led) {
            case LED_RED:
                REG(STM32_TIM1_CCR3) = ccr;
                break;

            case LED_BLUE:
                REG(STM32_TIM1_CCR4) = ccr;
                break;
        }
    }

    return result;
}

int led_set_intensity(int led, float intensity)
{
    int result = 0;

    if (led >= LED_SIZE || led < 0) {
        result = -1;
    }

    if (result == 0)
    {
        leds[led].intensity = intensity;
    }

    if (result == 0 && leds[led].active)
    {
        float i = 100 - leds[led].intensity;
        int ccr = (int)(UINT16_MAX * i / (float)100.0);
        led_set_ccr(led, ccr);
    }

    return result;
}

int led_on(int led)
{
    int result = 0;

    if (led >= LED_SIZE || led < 0) {
        result = -1;
    }

    if (result == 0)
    {
        leds[led].active = 1;
        led_set_intensity(led, leds[led].intensity);
    }

    return result;
}

int led_off(int led)
{
    int result = 0;

    if (led >= LED_SIZE || led < 0) {
        result = -1;
    }

    if (result == 0)
    {
        leds[led].active = 0;
        led_set_ccr(led, UINT16_MAX);
    }

    return result;
}
