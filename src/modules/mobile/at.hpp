#pragma once

#include <unistd.h>

#include <cstring>

#include <stm32.h>

#include <board_config.h>

#include "debug.hpp"
#include "repr.hpp"

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
ssize_t
exec_AT_verbose(Device & dev, FILE * out, const char cmd[], char buffer[], size_t size)
{
	fprintf(out, "%s\n", cmd);
	const ssize_t r = exec_AT(dev, cmd, buffer, size - 1);
	buffer[size - 1] = '\0';
	if (r == -1) { return -1; }

	buffer[r] = '\0';

	char repr_buffer[32];
	constexpr size_t repr_block = sizeof repr_buffer / 4;
	char * line, * tail;
	const char * sep = "\n\r";
	for (line = strtok_r(buffer, sep, &tail); line; line = strtok_r(nullptr, sep, &tail))
	{
		size_t len = strlen(line);
		size_t n;
		fprintf(out, ": ");
		while (repr_block < len)
		{
			n = repr_n(line, repr_block, repr_buffer) - repr_buffer;
			fwrite(repr_buffer, 1, n, out);
			line += repr_block;
			len -= repr_block;
		}
		n = repr_n(line, len, repr_buffer) - repr_buffer;
		fwrite(repr_buffer, 1, n, out);
		fprintf(out, "\n");
	}
	return r;
}

} // end of namespace bl600
