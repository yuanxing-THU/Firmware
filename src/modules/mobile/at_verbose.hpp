#pragma once

#include <cstdio>

#include "repr.hpp"

template <typename Device>
struct VerboseAT
{
	int n;
	Device & dev;
	int log;

	VerboseAT(Device & d, int l) : n(0), dev(d), log(l) {}
};

struct PrintableASCII
{
	bool
	operator() (char ch)
	{
		return ch == '\t' or (' ' <= ch and ch < '\x80');
	}
};

template <typename Device>
void __attribute__((noinline))
dump_line(VerboseAT<Device> & v, const char line[], size_t size)
{
	if (size > 0)
	{
		write(v.log, ": ", 2);
		write_repr<PrintableASCII, int>(v.log, line, size);
	}
}

template <typename Device>
void __attribute__((noinline))
dump_read(VerboseAT<Device> & v, const char buf[], size_t size)
{
	auto last = buf + size;
	auto first = buf;
	auto p = first;
	while (p != last)
	{
		if (*p =='\n' or *p == '\r')
		{
			dump_line(v, first, p - first);
			first = p;
			++first;
		}
		++p;
	}
	dump_line(v, first, last - first);
}

template <typename Device>
ssize_t
read(VerboseAT<Device> & v, void * buf, size_t size)
{
	const ssize_t r = read(v.dev, buf, size);
	if (r > 0) { dump_read(v, (const char *)buf, r); }
	return r;
}

template <typename Device>
inline void
dump_write(VerboseAT<Device> & v, const char buf[], size_t size)
{
	if (size == 1 and (*buf == '\n' or *buf == '\r')) { return; }

	char s[8];
	int s_len = snprintf(s, sizeof s, "%i# ", v.n);
	++v.n;

	if (s_len > 1)
	{
		write(v.log, s, s_len);
		write(v.log, buf, size);
		*s = '\n';
		write(v.log, s, 1);
	}
}

template <typename Device>
ssize_t
write(VerboseAT<Device> & v, const void * buf, size_t size)
{
	dump_write(v, (const char *)buf, size);
	return write(v.dev, buf, size);
}

template <typename Device>
VerboseAT<Device>
make_verbose_at(Device & dev, int log) { return VerboseAT<Device>(dev, log); }
