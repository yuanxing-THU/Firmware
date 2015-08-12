#pragma  once

#include <limits.h>

#include <sdlog2/directory.h>

#include "request_base.hpp"

template <>
struct Request< CMD_LOG_SET >
{
	using value_type = uint32_t;
};

template <typename Device>
void
reply(Request< CMD_LOG_SET >, const uint32_t current, Device & dev)
{
	char dir[PATH_MAX];
	uint32_t previous = current;
	if (previous == 0)
	{
		previous = 0xFFffFFff;
	}
	previous = sdlog2_dir_find_closest_number_lt(dir, previous, sdlog2_root);
	write(dev, &previous, sizeof previous);
}
