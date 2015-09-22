#include <nuttx/config.h>

extern "C" __EXPORT int main(int argc, const char *argv[]);

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <termios.h>

#include <systemlib/systemlib.h>

#include "at.hpp"
#include "dispatch.hpp"
#include "io_blocking.hpp"
#include "io_tty.hpp"
#include "read_write_log.hpp"
#include "unique_file.hpp"

#include "hack_nuttx_uart.hpp"

namespace
{

static bool daemon_should_run = false;
static bool daemon_running = false;

unique_file
open_serial(const char name[])
{
	unique_file d = tty_open(name);
	bool ok = ( fileno(d) != -1
		and tty_set_speed(fileno(d), B9600)
		and tty_use_ctsrts(fileno(d))
	);
	if (not ok)
	{
		auto true_errno = errno;
		dbg_perror("open_serial('%s')", name);
		close(d);
		errno = true_errno;
	}
	return d;
}

static int
daemon(int argc, char *argv[])
{
	fprintf(stderr, "%s starting...\n", argv[0]);

	unique_file d = open_serial(argv[1]);
	if (fileno(d) == -1)
	{
		perror(argv[0]);
		return 1;
	}

#ifdef ENABLE_DOG_DEBUG
	DevLog log (fileno(d), 2, "uart read  ", "uart write ");
#else
	auto & log = d;
#endif
	using namespace NuttxUART;
	auto dev = make_hack<WriteVariant::AS_MUCH_AS_POSSIBLE>(log);
	auto f = make_it_blocking< 1000/*ms*/ >(dev);

	FileWriteState write_state;

	daemon_running = true;
	fprintf(stderr, "%s started.\n", argv[0]);

	while (daemon_should_run)
		process_one_command(f, write_state);

	fprintf(stderr, "%s stopped.\n", argv[0]);
	daemon_running = false;

	return 0;
}

static bool
maintenance_allowed()
{
	if (daemon_running) { fprintf(stderr, "Stop the daemon first.\n"); }
	return not daemon_running;
}

static bool
exec_all_AT(const char devname[], int argc, const char * const arg[], char buf[], size_t size)
{
	using namespace bl600;

	if (not maintenance_allowed()) { return false; }

	unique_file serial = open_serial(devname);
	if (fileno(serial) == -1) { return false; }

	auto & log = serial;
	//DevLog log (fileno(serial), 2, "at read  ", "at write ");
	auto & dev = log;

	for (int i = 0; i < argc; ++i )
	{
		printf("%i# ", i);

		ssize_t r = exec_AT_verbose(dev, stdout, arg[i], buf, size);
		if (r == -1) { return false; }
	}

	return true;
}

static bool
exec_all_AT(const char devname[], int argc, const char * const arg[])
{
	char buf[64];
	memset(buf, 0, sizeof buf);
	return exec_all_AT(devname, argc, arg, buf, sizeof buf);
}

bool
parse_uint(const char s[], uint32_t &n, const char * & tail)
{
	char *p;
	n = std::strtoul(s, &p, 0);
	tail = p;
	return tail != s;
}

bool
parse_uint(const char s[], uint32_t &n)
{
	const char * tail = nullptr;
	bool ok = parse_uint(s, n, tail) and tail and *tail == '\0';
	if (not ok) { dbg("parse_uint('%s') failed.\n", s); }
	return ok;
}

static bool
version_firmware_parse(char destroyed_s[], unsigned (&v)[4])
{
	char * n, * tail;
	const char * sep = ".\n\r";

	size_t i = 0;
	n = strtok_r(destroyed_s, sep, &tail);
	while (n != nullptr)
	{
		if (not parse_uint(n, v[i])) { return false; }

		++i;
		if (i == 4) { return true; }

		n = strtok_r(nullptr, sep, &tail);
	}
	return false;
}

static bool
version_firmware_compare_le(const uint8_t (&min)[4], const unsigned (&ver)[4])
{
	bool ok;
	ok = min[0] < ver[0]
	 or (min[0] == ver[0] and (
		 min[1] < ver[1]
	     or (min[1] == ver[1] and (
		     min[2] < ver[2]
		 or (min[2] == ver[2] and (
			 min[3] <= ver[3]
		 ))
	     ))
	));
	return ok;
}

static bool
version_firmware_check(const char devname[])
{
	const uint8_t BL600_VERSION_FIRMWARE_MIN[] = { 1, 8, 88, 0 };
	const char * const at_i_3[] = { "AT I 3", nullptr };
	const char prefix[] = "10\t3\t";

	unsigned v4[4];
	char * p;
	char buf[32];
	memset(buf, 0, sizeof buf);

	bool ok = exec_all_AT(devname, 1, at_i_3, buf, sizeof buf);

	// 10\t3\tx.y.zz.q

	if (ok)
	{
		p = strstr(buf, prefix);
		dbg("version string: %s.\n", p);
		ok = p != nullptr;
	}

	if (ok)
	{
		p += strlen(prefix);
		ok = version_firmware_parse(p, v4);

		auto & m = BL600_VERSION_FIRMWARE_MIN;
		printf("required version: %u %u %u %u\n", m[0], m[1], m[2], m[3]);
		printf("modile's version: %u %u %u %u\n", v4[0], v4[1], v4[2], v4[3]);
	}

	if (ok)
	{
		ok = version_firmware_compare_le(BL600_VERSION_FIRMWARE_MIN, v4);
		if (ok) { printf("ready to work.\n"); }
		else { printf("upgrade required.\n"); }
	}

	return ok;
}


static bool
set_license(const char devname[], const char mac[], const char license[])
{
	if (strlen(mac) != 12)
	{
		fprintf(stderr, "Invalid MAC-12 '%s'.\n", mac);
		return 1;
	}
	if (strlen(license) != 20)
	{
		fprintf(stderr, "Invalid LICENCE-20 '%s'.\n", license);
		return 1;
	}

	const char * const at_i_14[] = { "AT I 14", nullptr };
	const char prefix_14[] = "10\t14\t01 ";

	char * p;
	char buf[64];
	memset(buf, 0, sizeof buf);

	bool ok = exec_all_AT(devname, 1, at_i_14, buf, sizeof buf);

	if (ok)
	{
		p = strstr(buf, prefix_14);
		dbg("reply mac: %s.\n", p);
		ok = p != nullptr;
	}

	if (ok)
	{
		p += strlen(prefix_14);
		p[12] = '\0';
		ok = strcasecmp(p, mac) == 0;
		dbg("mac matches %i.\n", ok);
	}

	if (ok)
	{
		char at_lic[32];
		int n = snprintf(at_lic, sizeof at_lic, "at+lic \"%s\"", license);
		ok = n == 29;
		dbg("cmd '%s' length %i -> %i.\n", at_lic, n, ok);
		const char * const exec_at_lic[] = { at_lic, nullptr };
		ok = ok and exec_all_AT(devname, 1, exec_at_lic, buf, sizeof buf);
	}

	return ok;
}

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

static void
usage(const char name[])
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\t%s start TTY\n", name);
	fprintf(stderr, "\t%s stop\n", name);
	fprintf(stderr, "\t%s status\n", name);
	fprintf(stderr, "\t%s mode at|default\n", name);
	fprintf(stderr, "\t%s at TTY command [command...]\n", name);
	fprintf(stderr, "\t%s firmware-version TTY\n", name);
	fprintf(stderr, "\t%s set-license TTY MAC-12 LICENCE-20\n", name);
	fprintf(stderr, "\n");
}

} // end of anonymous namespace

int
main(int argc, const char *argv[])
{
	if (argc < 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (argc == 3 and streq(argv[1], "start"))
	{
		if (daemon_running)
		{
			fprintf(stderr, "%s is already running.\n", argv[0]);
			return 1;
		}

		daemon_should_run = true;

		int r = task_spawn_cmd(argv[0],
				SCHED_DEFAULT,
				SCHED_PRIORITY_DEFAULT,
				CONFIG_TASK_SPAWN_DEFAULT_STACKSIZE,
				daemon,
				argv + 2);
		if (r == -1)
		{
			perror("task_spawn_cmd");
			return -1;
		}
	}
	else if (argc == 2 and streq(argv[1], "status"))
	{
		if (daemon_should_run) { printf("%s should run.\n", argv[0]); }
		else { printf("%s should NOT run.\n", argv[0]); }

		if (daemon_running) { printf("%s is running.\n", argv[0]); }
		else { printf("%s is NOT running.\n", argv[0]); }
	}
	else if (argc == 2 and streq(argv[1], "stop"))
	{
		if (not daemon_running)
		{
			fprintf(stderr, "%s is NOT running.\n", argv[0]);
			return 1;
		}
		daemon_should_run = false;
	}
	else if (argc == 3 and streq(argv[1], "mode"))
	{
		using namespace bl600;

		if (not maintenance_allowed()) { return 1; }

		if (streq(argv[2], "at"))
			mode_AT();
		else if (streq(argv[2], "default"))
			mode_default();
		else
		{
			usage(argv[0]);
			return 1;
		}
	}
	else if (argc > 3 and streq(argv[1], "at"))
	{
		bool ok = maintenance_allowed()
			and exec_all_AT(argv[2], argc - 3, argv + 3);
		if (not ok) { return 1; }
	}
	else if (argc == 3 and streq(argv[1], "firmware-version"))
	{
		bool ok = maintenance_allowed()
			and version_firmware_check(argv[2]);
		if (not ok) { return 1; }
	}
	else if (argc == 5 and streq(argv[1], "set-license"))
	{
		bool ok = maintenance_allowed()
			and set_license(argv[2], argv[3], argv[4]);
		if (not ok) { return 1; }
	}
	else
	{
		usage(argv[0]);
		return 1;
	}

	return 0;
}
