#pragma once

#include <sys/types.h>

#include "debug.hpp"

struct PrintableNone
{
	bool
	operator() (char) { return false; }
};

inline char
hex_digit(char x)
{
	x &= 0x0f;
	return x + (x < 10 ? '0' : 'a' - 10);
}

template <typename Printable, typename Device>
void
write_repr_char(Device &dev, char ch)
{
	Printable is_print;
	if (is_print(ch) and ch != '\\') { write(dev, (const void *)&ch, 1); }
	else
	{
		char buf[] = {'\\', 'x', hex_digit(ch >> 4), hex_digit(ch)};
		write(dev, buf, sizeof(buf));
	}
}

template <typename Printable, typename Device>
void
write_repr(const Device &dev, const void *buf, std::size_t buf_size)
{
	auto p = static_cast<const char *>(buf);
	for(std::size_t i = 0; i < buf_size; ++i)
		write_repr_char<Printable>(dev, p[i]);
	char nl = '\n';
	write(dev, &nl, 1);
}

template <typename Device>
void
write_repr(const Device &dev, const void *buf, std::size_t buf_size)
{ write_repr<PrintableNone, Device>(dev, buf, buf_size); }

template <typename InputIt, typename Size, typename OuputIt>
OuputIt
repr_n(InputIt first, Size n, OuputIt out)
{
	static_assert(sizeof(*first) == 1, "Only one byte types are suported.");

	if (n > 0)
	{
		for (size_t i = 0; i < n; ++i, ++first)
		{
			if ((' ' <= *first and *first < '\x80') or *first == '\t')
				*out++ = *first;
			else
			{
				*out++ = '\\';
				*out++ = 'x';
				*out++ = hex_digit(*first >> 4);
				*out++ = hex_digit(*first & 0xf);
			}
		}
	}

	return out;
}
