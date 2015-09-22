#pragma once

#include <cstring>
#include <unistd.h>

#include "debug.hpp"
#include "repr.hpp"

namespace bl600
{

bool
AT_OK(const char buffer[], size_t size)
{ return size >= 4 and strncmp(buffer + size - 4, "\n00\r", 4) == 0; }

struct AT_ReadReady
{
	bool
	operator () (const void * const buf, size_t size)
	{
		auto s = (const char *)buf;

		if (AT_OK(s, size)) { return true; }

		if (size < 4) { return false; }
		auto p = s + size - 4;
		while (p != s and *p != '\n') { --p; }
		return strncmp(p, "\n01\t", 4) == 0;
	}
};

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
		r = read(dev, buffer, size);
		if (r == -1) { dbg_perror("read response '%s'", cmd); }
	}
	return r;
}

template <typename Device, typename File>
bool
file_load(Device & dev, const char f_name[], File & f)
{
	const char WR_CMD[] = "AT+FWRH \"";

	char buf[32];
	char cmd[2 + sizeof WR_CMD + sizeof buf * 2];
	bool ok;
	ssize_t r;

	snprintf(cmd, sizeof cmd, "AT+FOW \"%s\"", f_name);
	r = exec_AT(dev, cmd, buf, sizeof buf);
	ok = r != -1 and AT_OK(buf, r);
	if (not ok) { return false; }

	strcpy(cmd, WR_CMD);
	char * const hex_first = cmd + strlen(WR_CMD);
	while (true)
	{
		r = read(f, buf, sizeof buf);

		if (r == -1)
		{
			dbg_perror("file_load(%s) read", f_name);
			return false;
		}

		if (r == 0) { break; }

		char * hex_last = hexify_n(buf, r, hex_first);
		hex_last[0] = '\"';
		hex_last[1] = '\0';

		r = exec_AT(dev, cmd, buf, sizeof buf);
		ok = r != -1 and AT_OK(buf, r);
		if (not ok) { return false; }
	}

	r = exec_AT(dev, "AT+FCL", buf, sizeof buf);
	ok = r != -1 and AT_OK(buf, r);
	if (not ok) { return false; }

	return true;
}

} // end of namespace bl600
