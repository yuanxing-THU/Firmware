#pragma once

namespace Activity {

const char ACTIVITY_FILE_DIR[] = "/fs/microsd/activity";
const int MAX_STR_LEN = 64;
const int MAX_PATH_LEN = 256;

const int ALLOWED_PARAM_COUNT = 22;
const int ACTIVITIES_COUNT = 5;

enum PARAM_LIMIT_TYPE { 
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
