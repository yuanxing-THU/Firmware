#pragma once

#include <stm32.h>

#include <board_config.h>

#include "debug.hpp"

namespace bl600
{

void
reset()
{
#ifdef GPIO_BL600_RESET
	dbg("bl600 resetting...\n");
	stm32_gpiowrite(GPIO_BL600_RESET, 0);
	sleep(3);
	stm32_gpiowrite(GPIO_BL600_RESET, 1);
	sleep(3);
	dbg("bl600 reset done...\n");
#endif
}

void
mode_AT()
{
#ifdef GPIO_BL600_RESET
	stm32_gpiowrite(GPIO_BL600_SIO_07, 0);
	stm32_gpiowrite(GPIO_BL600_SIO_28, 1);
	reset();
#endif
}

void
mode_default()
{
#ifdef GPIO_BL600_RESET
	stm32_configgpio(GPIO_BL600_SIO_07);
	stm32_configgpio(GPIO_BL600_SIO_28);
	reset();
#endif
}

} // end of namespace bl600
