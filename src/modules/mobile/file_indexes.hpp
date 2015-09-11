#pragma once

#include <limits.h>

#include <activity/activity_files.hpp>
#include <sdlog2/directory.h>

#include "protocol.h"


/*
 * Index structure.
 */

enum FileCatalog : uint8_t
{
	INVALID,
	ACTIVITY,
	PUBLIC,
	LOGS,
};

FileCatalog
catalog(file_index_t index)
{
	if (index == 1) { return FileCatalog::PUBLIC; }
	if ((index >> 16) == 0x0001) { return FileCatalog::ACTIVITY; }
	if ((index >> 31) == 1) { return FileCatalog::LOGS; }
	return FileCatalog::INVALID;
}

using filename_buf_t = char[PATH_MAX];

/*
 * Activity files.
 */

void
parse_activity_index(file_index_t index, uint8_t & activity, uint8_t & attribute)
{
	activity = (index >> 8) & 0xFF;
	attribute = index & 0xFF;
}

bool
get_activity_filename(file_index_t index, filename_buf_t & pathname)
{
	using namespace AirDog;
	uint8_t activity, attribute;
	parse_activity_index(index, activity, attribute);
	return Activity::Files::get_path(activity, attribute, pathname);
}

bool
is_activity_index_valid(file_index_t index)
{
	using namespace AirDog;
	uint8_t activity, attribute;
	parse_activity_index(index, activity, attribute);
	return Activity::Files::has_valid_id(activity, attribute);
}

bool
is_activity_file_valid(const char tmp_path[], file_index_t index)
{
	using namespace AirDog;
	uint8_t activity, attribute;
	parse_activity_index(index, activity, attribute);
	return Activity::Files::has_valid_content(activity, attribute, tmp_path);
}


/*
 * Log files.
 */

void
parse_log_index(file_index_t index, uint32_t & dir_no, uint32_t & file_no)
{
	index &= ~0x80000000;
	dir_no = index >> 8;
	file_no = index & 0xFF;
}

bool
is_log_index_valid(file_index_t index)
{
	uint32_t dir_no, file_no;
	parse_log_index(index, dir_no, file_no);
	return (file_no < SDLOG2_FILE_KIND_MAX);
}

bool
get_log_filename(file_index_t index, filename_buf_t & name)
{
	uint32_t dir_no, file_no;
	parse_log_index(index, dir_no, file_no);

	static_assert(sizeof name == PATH_MAX,
			"sdlog2_filename requires PATHMAX");

	char dir[PATH_MAX];
	bool ok = file_no < SDLOG2_FILE_KIND_MAX
		and sdlog2_dir_find_by_number(dir, dir_no, sdlog2_root)
		and sdlog2_filename(name, dir, sdlog2_file_kind_t(file_no));

	return ok;
}


/*
 * General files.
 */

bool
get_filename(file_index_t index, filename_buf_t & name)
{
	FileCatalog c = catalog(index);
	bool ok;
	switch (c)
	{
	case FileCatalog::ACTIVITY:
		ok = get_activity_filename(index, name);
		break;
	case FileCatalog::PUBLIC:
		strncpy(name, "/fs/microsd/mobile/public.dat", sizeof name);
		ok = index == 1;
		break;
	case FileCatalog::LOGS:
		ok = get_log_filename(index, name);
		break;
	default:
		ok = false;
	}
	if (ok) { dbg("File index 0x%08x name '%s'.\n", index, name); }
	else
	{
		*name = '\0';
		dbg("No name for file index 0x%08x.\n", index);
	}
	return ok;
}

bool
is_file_index_valid(file_index_t index)
{
	FileCatalog c = catalog(index);
	return (c == FileCatalog::ACTIVITY and is_activity_index_valid(index))
		or (c == FileCatalog::LOGS and is_log_index_valid(index))
		or (c != FileCatalog::INVALID);
}

bool
is_file_writable(file_index_t index)
{
	FileCatalog c = catalog(index);
	return c == FileCatalog::ACTIVITY or c == FileCatalog::PUBLIC;
}

bool
is_file_content_valid(const char tmp_path[], file_index_t index)
{
	FileCatalog c = catalog(index);
	switch (c)
	{
	case FileCatalog::ACTIVITY:
		return is_activity_file_valid(tmp_path, index);
	case FileCatalog::PUBLIC:
		return true;
	default:
		return false;
	}
}
