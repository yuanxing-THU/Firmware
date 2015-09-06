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

	DevLog log (fileno(d), 2, "uart read  ", "uart write ");
	auto f = make_it_blocking< 1000/*ms*/ >(log);

	FileWriteState write_state;
	StatusOverall status;

	daemon_running = true;
	fprintf(stderr, "%s started.\n", argv[0]);

	while (daemon_should_run)
		process_one_command(f, write_state, status);

	fprintf(stderr, "%s stopped.\n", argv[0]);
	daemon_running = false;

	return 0;
}

static bool
exec_all_AT(const char devname[], int argc, const char * const arg[])
{
	using namespace bl600;

	if (daemon_running)
	{
		fprintf(stderr, "Stop the daemon first.\n");
		return false;
	}

	unique_file serial = open_serial(devname);
	if (fileno(serial) == -1) { return false; }

	auto & log = serial;
	//DevLog log (fileno(serial), 2, "at read  ", "at write ");
	auto & dev = log;

	char buf[32];
	memset(buf, 0, sizeof buf);

	for (int i = 0; i < argc; ++i )
	{
		printf("%i# ", i);

		ssize_t r = exec_AT_verbose(dev, stdout, arg[i], buf, sizeof buf);
		if (r == -1) { return false; }
	}

	return true;
}

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

static void
usage(const char name[])
{
	fprintf(stderr,
		"Usage: %s start TTY\n"
		"       %s stop\n"
		"       %s status\n"
		"       %s at TTY command [command...]\n"
		"       %s firmware-version TTY\n"
		"\n",
		name, name, name, name, name
	);
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
		bool ok = exec_all_AT(argv[2], argc - 3, argv + 3);
		if (not ok) { return 1; }
	}
	else if ((argc == 3 or argc == 4) and streq(argv[1], "firmware-version"))
	{
		static const char * const at[] = { "AT I 3", nullptr };
		bool ok = exec_all_AT(argv[2], 1, at);
		// 10\t3\tx.y.zz.q
		if (not ok) { return 1; }
	}
	else
	{
		usage(argv[0]);
		return 1;
	}

	return 0;
}
