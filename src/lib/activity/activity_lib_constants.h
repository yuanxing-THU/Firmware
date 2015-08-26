#pragma once

namespace Activity {
const char ACTIVITY_FILE_DIR[PATH_MAX] = "/fs/microsd/activity";
const int MAX_STR_LEN = 64;
const int MAX_PATH_LEN = 256;

const int ALLOWED_PARAM_COUNT = 14;
const int ACTIVITIES_COUNT = 10;

enum PARAM_LIMIT_KIND { 
    INVISIBLE = 0,
    STATIC,
    INTERVAL,
    VALUES_INT,
    VALUES_FLOAT,
    VALUES_STR
};

enum ACTIVITY_FILE_TYPE{
    PARAMS = 0,
    ICON
};

}
//end of namespace Activity
