#include <stdio.h>
#include <string.h>

#include "activity_config_list.hpp"

const char* activityNames[] =
{
    "Demo",      // ACTIVITY_TEST
    "Surf",      // ACTIVITY_SURF
    "Ski",       // ACTIVITY_SKI
    "Skatepark", // ACTIVITY_SKATE
    "MTB",       // ACTIVITY_CYCLE
    "Wakeboard", // ACTIVITY_WAKE
    "Motocross", // ACTIVITY_BIKE
    "Snow",      // ACTIVITY_SNOWBOARD
    "Kiteboard", // ACTIVITY_KITE
    "Custom",    // ACTIVITY_CUSTOM
    nullptr      // ACTIVITY_MAX
};

namespace Activity {

bool activity_config_list_inited = false;
ActivityConfig ACTIVITY_CONFIG_LIST[ ACTIVITIES_COUNT+1 ];
ActivityConfig ACTIVITY_CONFIG_BASE;

ParamConfig::ParamConfig (){
    p_id = -1;
    limit_kind = -1;
    value_num = 0;
    default_value = 0.0f;
};

ParamConfig::ParamConfig(const char * _name, int _limit_kind, int _target_device, float _default_value, float _istart, float _iend, float _step, std::initializer_list <float> _values) {

    p_id = -1;

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++)
        if (strcmp(ALLOWED_PARAMS[i].name, _name) == 0 && _target_device == ALLOWED_PARAMS[i].target_device)
            p_id = ALLOWED_PARAMS[i].id;

    if (p_id == -1) {

        // TODO: change to proper debug output function
        printf("Parameter: %s with target device type %d not allowed.\n", _name, _target_device);
        fflush(stdout);
    }

    limit_kind = _limit_kind;

    default_value = _default_value;

    istart = _istart;
    iend = _iend;
    step = _step;
    value_num = _values.size();

    values = new float[_values.size()];

    int i = 0;
    for (auto it : _values)  {
        values[i] = it;
        ++i;
    }
};

ParamConfig::~ParamConfig(){
    delete [] values;
};

ParamConfig&
ParamConfig::operator = (const ParamConfig &a) {

    p_id = a.p_id;
    limit_kind = a.limit_kind;

    istart = a.istart;
    iend = a.iend;

    step = a.step;
    value_num = a.value_num;

    default_value = a.default_value;

    delete [] values;
    values = new float[value_num];

    for (int i=0;i<value_num;i++)
        values[i] = a.values[i];

    return *this;

}

ParamConfig::ParamConfig(const ParamConfig &a) {
    *this = a;
}

const char *
ParamConfig::get_name(){
    return ALLOWED_PARAMS[p_id].name;
};

int
ParamConfig::get_target_device(){
    return ALLOWED_PARAMS[p_id].target_device;
}

const char *
ParamConfig::get_param_name(){
    return ALLOWED_PARAMS[p_id].name;
}

const char *
ParamConfig::get_display_name(){
    return ALLOWED_PARAMS[p_id].display_name;
}

ActivityConfig::ActivityConfig(){
    id = -1;
    name = nullptr;
    param_count = 0;
    params = nullptr;
};

ActivityConfig::ActivityConfig(int _param_count){
    param_count = _param_count;
    params = new ParamConfig[param_count];
}

ActivityConfig::ActivityConfig(int _id, const char * _name, std::initializer_list <ParamConfig> _params ){

    printf("Creating activity %.2f\n", (double)_id);

    id = _id;
    name = _name;

    param_count = _params.size();
    params = new ParamConfig[param_count];

    int i = 0;
    for (auto p : _params) {
        params[i] = p;
        ++i;
    }
}

ActivityConfig::~ActivityConfig() {
   delete [] params;
}


ActivityConfig&
ActivityConfig::operator=(ActivityConfig &&a) {

    id = a.id;
    param_count = a.param_count;
    name = a.name;

    delete [] params;

    params = a.params;
    a.params = nullptr;

    return *this;
}

ParamConfig *
get_activity_param_config(int activity, int param) {

    if (!allowed_params_inited) init_allowed_params();
    if (!activity_config_list_inited) init_activity_config_list();

    if (activity >= ACTIVITIES_COUNT) return nullptr;
    if (param >= ALLOWED_PARAM_COUNT) return nullptr;

    for (int j=0;j<ACTIVITY_CONFIG_LIST[activity].param_count;j++){

        if (param == ACTIVITY_CONFIG_LIST[activity].params[j].p_id) {
            return &ACTIVITY_CONFIG_LIST[activity].params[j];
        }
    }

    return &ACTIVITY_CONFIG_BASE.params[param];
}

int
init_activity_config_list(){

    if (activity_config_list_inited) return true;

    ACTIVITY_CONFIG_BASE = ActivityConfig(
        999,
        "Base",
        {
            //           Param name           Type         Device  Default Istart Iend  Step  Values
            ParamConfig("A_ACTIVITY",         INVISIBLE,     ALL,    0,      -1,    -1,   -1,   {}                 ),
            ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,    1,      -1,    -1,   -1,   {0,1,2,3,4}        ),
            ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,    DOG,    1,      -1,    -1,   -1,   {1,2}              ),
            ParamConfig("RTL_RET_ALT",        INTERVAL,      DOG,    20.0f,  10,   100,    5,   {}                 ),
            ParamConfig("NAV_TAKEOFF_ALT",    INTERVAL,      DOG,    8.0f,    5,    50,    1,   {}                 ),
            ParamConfig("A_INIT_POS_U",       VALUES_STR,    DOG,    0,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("A_INIT_POS_D",       INTERVAL,      DOG,    8.0f,    5,   100,    1,   {}                 ),
            ParamConfig("V_REACTION",         VALUES_STR,    DOG,    2,      -1,    -1,   -1,   {0,1,2}            ),
            ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,    1,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("SENS_SON_MIN",       INTERVAL,      DOG,    5.0f,    3,    20, 0.5f,   {}                 ),
            ParamConfig("PAFOL_OPT_D",        INVISIBLE,      DOG,    12.0f,   5,    40,    1,   {}                 ),
            ParamConfig("AIRD_TRAJ_RAD",      INVISIBLE,     ALL,    5.0f,    3,    20,    1,   {}                 ),
            ParamConfig("FOL_LPF_XY",         INVISIBLE,     DOG,    1.0f,    0,    2, 0.05f,   {}                 ),
            ParamConfig("FOL_VEL_FF_XY",      INVISIBLE,     DOG,    0.7f,    0,    2, 0.05f,   {}                 ),
            ParamConfig("BAT_WARN_LVL",       INVISIBLE,     DOG,    0.2f,    0,    1, 0.05f,   {}                 ),
            ParamConfig("BAT_CRIT_LVL",       INVISIBLE,     DOG,    0.1f,    0,    1, 0.05f,   {}                 ),
            ParamConfig("A_SAH_NO_SPOT",      INVISIBLE,     DOG,    0,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("OFF_FR_ADD_ANG",     INTERVAL,      DOG,    0,      0.00f,   6.28318f,  0.78539f,   {0,1} ), // TODO: VIRTUAL PARAM
            ParamConfig("CBP_MAX_INIT_SPD",   INTERVAL,      DOG,    5,       1,    10,    1,   {}                 ),
            ParamConfig("A_FOL_IMDTLY",       VALUES_STR,     DOG,    0,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("FOL_FF_GRAD_STRT",   INVISIBLE,      DOG,    20,      0,   150,    5,   {}                 ),
            ParamConfig("FOL_FF_GRAD_END",    INVISIBLE,      DOG,    150,    10,   200,    5,   {}                 ),
            ParamConfig("FOL_FF_GRAD_USE",    INVISIBLE,    DOG,    1,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("FOL_USE_ALT",        INVISIBLE,    DOG,    1,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("INAV_INIT_EPH",      INVISIBLE,     DOG,    0,       1,     5, 0.1f,   {}                 ),
            ParamConfig("INAV_INIT_EPV",      INVISIBLE,     DOG,    0,       1,    10, 0.1f,   {}                 ),
            ParamConfig("INAV_INIT_WAIT",     INVISIBLE,     DOG,    0,    4000,300000, 1000,   {}                 ),
            ParamConfig("MPC_PITCH_LPF",      INVISIBLE,     DOG,    50,      0,   500,    5,   {}                 ),
            ParamConfig("MPC_XY_FF",          INVISIBLE,     DOG,    1,       0,    1,   0.05f, {}                 ),
            ParamConfig("MPC_XY_P",           INVISIBLE,     DOG,    0.5f,    0,    1,   0.05f, {}                 ),
            ParamConfig("MPC_XY_VEL_D",       INVISIBLE,     DOG,    0.0007f,  0,    1,  0.0001f,   {}                 ),
            ParamConfig("MPC_XY_VEL_I",       INVISIBLE,     DOG,    0.0015f,  0,    1,  0.0001f,   {}                 ),
            ParamConfig("MPC_XY_VEL_MAX",     INVISIBLE,     DOG,    20,      5,     25,    1,     {}                 ),
            ParamConfig("MPC_XY_VEL_P",       INVISIBLE,     DOG,    0.1f,    0,     6,  0.05f,    {}                 ),
            ParamConfig("MPC_Z_FF",           INVISIBLE,     DOG,    0.3f,    0,     1,  0.05f,    {}                 ),
            ParamConfig("MPC_Z_P",            INVISIBLE,     DOG,    1.1f,    0,     6,  0.05f,    {}                 ),
            ParamConfig("MPC_Z_VEL_D",        INVISIBLE,     DOG,    0.01f,   0,     5,  0.01f,   {}                 ),
            ParamConfig("MPC_Z_VEL_I",        INVISIBLE,     DOG,    0.05f,   0,     5,  0.01f,   {}                 ),
            ParamConfig("MPC_Z_VEL_MAX",      INVISIBLE,     DOG,    0.5f,    1,     6,   0.5f,   {}                 ),
            ParamConfig("MPC_Z_VEL_P",        INVISIBLE,     DOG,    0.28f,   0,     5,  0.01f,   {}               ),
            ParamConfig("PAFOL_GT_AC_DST",    INVISIBLE,     DOG,    2,       1,    10,    1,   {}                 ),
            ParamConfig("SDLOG_ON_BOOT",      INVISIBLE,     DOG,    1,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("SENS_SON_ON",        INVISIBLE,     DOG,    0,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("UBX_ENABLE_SBAS",    INVISIBLE,     DOG,    1,      -1,    -1,   -1,   {0,1}              ),
            ParamConfig("UBX_GPS_MAX_CHN",    INVISIBLE,     DOG,    16,      8,    24,    1,   {}                 ),
            ParamConfig("UBX_GPS_MIN_CHN",    INVISIBLE,     DOG,    8,       1,    16,    1,   {}                 ),
            ParamConfig("INAV_W_Z_GPS_P",     INVISIBLE,     DOG,    0.005,   0,     1,  0.001f,{}                 ),
            ParamConfig("INAV_W_Z_BARO",      INVISIBLE,     DOG,    0.5,     0,     1,   0.05f,{}                 ),
            ParamConfig("NAV_CP_FIR_AL",      INVISIBLE,     DOG,   10,     -1,    -1,  -1,    {}                 ),
            ParamConfig("NAV_CP_FIR_LA",      INVISIBLE,     DOG,   569496304,     -1,    -1,  -1,    {}                 ),
            ParamConfig("NAV_CP_FIR_LO",      INVISIBLE,     DOG,   244123330,     -1,    -1,  -1,    {}                 ),
            ParamConfig("NAV_CP_LAS_AL",      INVISIBLE,     DOG,   10,     -1,    -1,  -1,    {}                 ),
            ParamConfig("NAV_CP_LAS_LA",      INVISIBLE,     DOG,   0,     -1,    -1,  -1,    {}                 ),
            ParamConfig("NAV_CP_LAS_LO",      INVISIBLE,     DOG,   0,     -1,    -1,  -1,    {}                 ),

        });

    ACTIVITY_CONFIG_LIST[0] = ActivityConfig(
            ACTIVITY_TEST,
            activityNames[ACTIVITY_TEST],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {0,1,4}            ),
                ParamConfig("A_SAH_NO_SPOT",      VALUES_STR,  DOG,    0,      -1,    -1,   -1,   {0,1}              ),
            });

    ACTIVITY_CONFIG_LIST[1] = ActivityConfig(
            ACTIVITY_SURF,
            activityNames[ACTIVITY_SURF],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,  DOG,    3,      -1,    -1,   -1,   {3}                ),
                ParamConfig("A_INIT_POS_U",       VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {1}                ),
                ParamConfig("BAT_WARN_LVL",       INVISIBLE,   DOG,    0.3f,    0,    1, 0.05f,   {}                 ),
                ParamConfig("BAT_CRIT_LVL",       INVISIBLE,   DOG,    0.15f,   0,    1, 0.05f,   {}                 ),
            });

    ACTIVITY_CONFIG_LIST[2] = ActivityConfig(
            ACTIVITY_SKI,
            activityNames[ACTIVITY_SKI],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {1}                 ),
                ParamConfig("RTL_RET_ALT",        INTERVAL,    DOG,    35.0f,  10,   100,    5,   {}                 ),
                ParamConfig("A_INIT_POS_U",       VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {0,1}               ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {1}                 ),
                ParamConfig("SENS_SON_MIN",       INTERVAL,    DOG,    7.0f,    3,    20, 0.5f,   {}                  ),
            });

    ACTIVITY_CONFIG_LIST[3] = ActivityConfig(
            ACTIVITY_SKATE,
            activityNames[ACTIVITY_SKATE],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,      DOG,    0,      -1,    -1,   -1,   {0}                ),
                ParamConfig("A_INIT_POS_U",       VALUES_STR,      DOG,    1,      -1,    -1,   -1,   {1}                ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,  DOG,    0,      -1,    -1,   -1,   {0}                ),
                ParamConfig("BAT_WARN_LVL",       INVISIBLE,   DOG,    0.3f,    0,    1, 0.05f,   {}                 ),
                ParamConfig("BAT_CRIT_LVL",       INVISIBLE,   DOG,    0.15f,   0,    1, 0.05f,   {}                 ),
            });

    ACTIVITY_CONFIG_LIST[4] = ActivityConfig(
            ACTIVITY_CYCLE,
            activityNames[ACTIVITY_CYCLE],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,  DOG,    1,       -1,    -1,   -1,   {1}                ),
                ParamConfig("RTL_RET_ALT",        INTERVAL,    DOG,    35.0f,  10,   100,    5,   {}                 ),
                ParamConfig("A_INIT_POS_U",       VALUES_STR,  DOG,    1,       -1,    -1,   -1,   {0,1}              ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,  DOG,    1,       -1,    -1,   -1,   {1}                ),
                ParamConfig("SENS_SON_MIN",       INTERVAL,    DOG,    7.0f,     3,    20, 0.5f,   {}                 ),
            });

    ACTIVITY_CONFIG_LIST[5] = ActivityConfig(
            ACTIVITY_WAKE,
            activityNames[ACTIVITY_WAKE],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,  DOG,    0,      -1,    -1,   -1,   {0,1,2,3,4}        ),
                ParamConfig("A_INIT_POS_U",       VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,  DOG,    0,      -1,    -1,   -1,   {1}                ),
                ParamConfig("BAT_WARN_LVL",       INVISIBLE,   DOG,    0.3f,    0,    1, 0.05f,   {}                 ),
                ParamConfig("BAT_CRIT_LVL",       INVISIBLE ,  DOG,    0.15f,   0,    1, 0.05f,   {}                 ),
            });

    ACTIVITY_CONFIG_LIST[6] = ActivityConfig(
            ACTIVITY_BIKE,
            activityNames[ACTIVITY_BIKE],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("A_INIT_POS_U",       VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("RTL_RET_ALT",        INTERVAL,    DOG,    35.0f,  10,   100,    5,   {}                 ),
                ParamConfig("SENS_SON_MIN",       INTERVAL,    DOG,    7.0f,    3,    20, 0.5f,   {}                 ),
                ParamConfig("AIRD_TRAJ_RAD",      INVISIBLE,   ALL,    6.0f,    3,    20,    1,   {}                 ),
            });

    ACTIVITY_CONFIG_LIST[7] = ActivityConfig(
            ACTIVITY_SNOWBOARD,
            activityNames[ACTIVITY_SNOWBOARD],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {1}                 ),
                ParamConfig("RTL_RET_ALT",        INTERVAL,    DOG,    35.0f,  10,   100,    5,   {}                 ),
                ParamConfig("A_INIT_POS_U",       VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {0,1}               ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,  DOG,    1,      -1,    -1,   -1,   {1}                 ),
                ParamConfig("SENS_SON_MIN",       INTERVAL,    DOG,    7.0f,    3,    20, 0.5f,   {}                  ),
            });

    ACTIVITY_CONFIG_LIST[8] = ActivityConfig(
            ACTIVITY_KITE,
            activityNames[ACTIVITY_KITE],
            {
            }
            );

    ACTIVITY_CONFIG_LIST[9] = ActivityConfig(
            ACTIVITY_CUSTOM,
            activityNames[ACTIVITY_CUSTOM],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("A_ACTIVITY",         INVISIBLE,    ALL,    0,      -1,    -1,   -1,   {}                 ),
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,   DOG,    1,      -1,    -1,   -1,   {0,1,2,3,4}        ),
                ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,   DOG,    1,      -1,    -1,   -1,   {1,2}              ),
                ParamConfig("RTL_RET_ALT",        INTERVAL,     DOG,    20.0f,  10,   100,    5,   {}                 ),
                ParamConfig("NAV_TAKEOFF_ALT",    INTERVAL,     DOG,    8.0f,    5,    50,    1,   {}                 ),
                ParamConfig("A_INIT_POS_U",       VALUES_STR,   DOG,    0,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("A_INIT_POS_D",       INTERVAL,     DOG,    8.0f,    5,   100,    1,   {}                 ),
                ParamConfig("V_REACTION",         VALUES_STR,   DOG,    2,      -1,    -1,   -1,   {0,1,2}            ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,   DOG,    1,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("SENS_SON_MIN",       INTERVAL,     DOG,    5.0f,    3,    20, 0.5f,   {}                 ),
                ParamConfig("AIRD_TRAJ_RAD",      INTERVAL,     ALL,    5.0f,    3,    20,    1,   {}                 ),
                ParamConfig("FOL_LPF_XY",         INTERVAL,     DOG,    1.0f,    0,    2, 0.05f,   {}                 ),
                ParamConfig("FOL_VEL_FF_XY",      INTERVAL,     DOG,    0.7f,    0,    2, 0.05f,   {}                 ),
                ParamConfig("BAT_WARN_LVL",       INTERVAL,     DOG,    0.2f,    0,    1, 0.05f,   {}                 ),
                ParamConfig("BAT_CRIT_LVL",       INTERVAL,     DOG,    0.1f,    0,    1, 0.05f,   {}                 ),
                ParamConfig("A_SAH_NO_SPOT",      VALUES_STR,   DOG,    0,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("OFF_FR_ADD_ANG",     INTERVAL,     DOG,    0,      0.00f,   6.28318f,  0.78539f,   {0,1} ), // TODO: VIRTUAL PARAM
                ParamConfig("CBP_MAX_INIT_SPD",   INTERVAL,     DOG,    5,       1,    10,    1,   {}                 ),
                ParamConfig("A_FOL_IMDTLY",       VALUES_STR,   DOG,    0,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("FOL_FF_GRAD_STRT",   INTERVAL,     DOG,    20,      0,   150,    5,   {}                 ),
                ParamConfig("FOL_FF_GRAD_END",    INTERVAL,     DOG,    150,    10,   200,    5,   {}                 ),
                ParamConfig("FOL_FF_GRAD_USE",    VALUES_STR,   DOG,    1,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("FOL_USE_ALT",        VALUES_STR,   DOG,    1,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("INAV_INIT_EPH",      INTERVAL,     DOG,    0,       1,     5, 0.1f,   {}                 ),
                ParamConfig("INAV_INIT_EPV",      INTERVAL,     DOG,    0,       1,    10, 0.1f,   {}                 ),
                ParamConfig("INAV_INIT_WAIT",     INTERVAL,     DOG,    0,    4000,300000, 1000,   {}                 ),
                ParamConfig("MPC_PITCH_LPF",      INTERVAL,     DOG,    50,      0,   500,    5,   {}                 ),
                ParamConfig("MPC_XY_FF",          INTERVAL,     DOG,    1,       0,    1,   0.05f, {}                 ),
                ParamConfig("MPC_XY_P",           INTERVAL,     DOG,    0.5f,    0,    1,   0.05f, {}                 ),
                ParamConfig("MPC_XY_VEL_D",       INTERVAL_HP,  DOG,    0.0007f,  0,    1,  0.0001f,   {}                 ),
                ParamConfig("MPC_XY_VEL_I",       INTERVAL_HP,  DOG,    0.0015f,  0,    1,  0.0001f,   {}                 ),
                ParamConfig("MPC_XY_VEL_MAX",     INTERVAL,     DOG,    20,      5,     25,    1,     {}                 ),
                ParamConfig("MPC_XY_VEL_P",       INTERVAL,     DOG,    0.1f,    0,     6,  0.05f,    {}                 ),
                ParamConfig("MPC_Z_FF",           INTERVAL,     DOG,    0.3f,    0,     1,  0.05f,    {}                 ),
                ParamConfig("MPC_Z_P",            INTERVAL,     DOG,    1.1f,    0,     6,  0.05f,    {}                 ),
                ParamConfig("MPC_Z_VEL_D",        INTERVAL,     DOG,    0.01f,   0,     5,  0.01f,   {}                 ),
                ParamConfig("MPC_Z_VEL_I",        INTERVAL,     DOG,    0.05f,   0,     5,  0.01f,   {}                 ),
                ParamConfig("MPC_Z_VEL_MAX",      INTERVAL,     DOG,    0.5f,    1,     6,   0.5f,   {}                 ),
                ParamConfig("MPC_Z_VEL_P",        INTERVAL,     DOG,    0.28f,   0,     5,  0.01f,   {}               ),
                ParamConfig("SDLOG_ON_BOOT",      VALUES_STR,   DOG,    1,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("SENS_SON_ON",        VALUES_STR,   DOG,    0,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("UBX_ENABLE_SBAS",    VALUES_STR,   DOG,    1,      -1,    -1,   -1,   {0,1}              ),
                ParamConfig("UBX_GPS_MAX_CHN",    INTERVAL,     DOG,    16,      8,    24,    1,   {}                 ),
                ParamConfig("UBX_GPS_MIN_CHN",    INTERVAL,     DOG,    8,       1,    16,    1,   {}                 ),
                ParamConfig("INAV_W_Z_GPS_P",     INTERVAL_HP,  DOG,    0.005,   0,     1,  0.001f,{}                 ),
                ParamConfig("INAV_W_Z_BARO",      INTERVAL,     DOG,    0.5,     0,     1,   0.05f,{}                 ),
            });

    activity_config_list_inited = true;
    return true;

    };

}
//End of namepsace Activity
