#pragma once

#ifdef ENABLE_DOG_DEBUG

#include <syslog.h>
#include <cerrno>

#define DOG_PRINT(...) do {                              \
		const int save_errno = errno;            \
		syslog(__VA_ARGS__);            \
		errno = save_errno;			 \
	} while (0)

#else

/*
 * Prevent unused-variable errors and warnings while debug is disabled.
 *
 * With gcc 4.7 empty function is more effective than
 *
 *        if(ENABLE_DOG_DEBUG) {...}
 *
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"

static inline void
DOG_PRINT(const char fmt[], ...) {}

#pragma GCC diagnostic pop

#endif
