#pragma once

#include <unistd.h>

#include <stm32.h>

#include <board_config.h>

#include "debug.hpp"

namespace bl600
{

void
reset()
{
	dbg("bl600 resetting...\n");
	stm32_gpiowrite(GPIO_BL600_RESET, 0);
	sleep(3);
	stm32_gpiowrite(GPIO_BL600_RESET, 1);
	sleep(3);
	dbg("bl600 reset done...\n");
}

void
mode_AT()
{
	stm32_gpiowrite(GPIO_BL600_SIO_07, 0);
	stm32_gpiowrite(GPIO_BL600_SIO_28, 1);
	reset();
}

void
mode_default()
{
	stm32_configgpio(GPIO_BL600_SIO_07);
	stm32_configgpio(GPIO_BL600_SIO_28);
	reset();
}

template <typename Device>
ssize_t
exec_AT(Device & dev, const char cmd[], char buffer[], size_t size)
{
	const char CR = '\r';
	size_t len = strlen(cmd);

	ssize_t r = write(dev, cmd, len);
	if (r != -1) { r = write(dev, &CR, 1); }

	if (r == -1) { dbg_perror("write('%s')", cmd); }
	else
	{
		usleep(100 * 1000 /*µs*/);
		r = read(dev, buffer, size);
		if (r == -1) { dbg_perror("read response '%s'", cmd); }
	}
	return r;
}

template <typename Device>
bool
load_file(Device & dev, int f)
{
	return false;
}

} // end of namespace bl600
