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
ActivityConfig ACTIVITY_CONFIG_GOD;

ParamConfig::ParamConfig (){
    p_id = -1;
    limit_kind = -1;
    value_num = 0;
};

ParamConfig::ParamConfig(const char * _name, int _limit_kind, int _target_device, float _istart, float _iend, float _step, std::initializer_list <float> _values) {

    p_id = -1;
    p_idx = -1;

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++)
        if (strcmp(ALLOWED_PARAMS[i].name, _name) == 0 && _target_device == ALLOWED_PARAMS[i].target_device) {
            p_id = ALLOWED_PARAMS[i].id;
            p_idx = i;
        }

    if (p_id == -1) {
        printf("Parameter: %s with target device type %d not allowed.\n", _name, _target_device);
        fflush(stdout);
    }

    limit_kind = _limit_kind;

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
    p_idx = a.p_idx;
    limit_kind = a.limit_kind;

    istart = a.istart;
    iend = a.iend;

    step = a.step;
    value_num = a.value_num;

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
    return ALLOWED_PARAMS[p_idx].name;
};

int
ParamConfig::get_target_device(){
    return ALLOWED_PARAMS[p_idx].target_device;
}

const char *
ParamConfig::get_param_name(){
    return ALLOWED_PARAMS[p_idx].name;
}

const char *
ParamConfig::get_display_name(){
    return ALLOWED_PARAMS[p_idx].display_name;
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
get_activity_param_config(int activity, int param_idx) {
    

    if (!allowed_params_inited) init_allowed_params();
    if (!activity_config_list_inited) init_activity_config_list();

    int activity_god_mode = 0;

    param_get(param_find("A_ACTIVITY_GOD"), &activity_god_mode);

    if (activity > ACTIVITIES_COUNT) return nullptr;

    int param_id = ALLOWED_PARAMS[param_idx].id;
    ParamConfig * ptr = nullptr;

    for (int p=0;p<ACTIVITY_CONFIG_BASE.param_count;p++){
        if (ACTIVITY_CONFIG_BASE.params[p].p_id == param_id) {
            ptr = &ACTIVITY_CONFIG_BASE.params[p];
        }
    }

    for (int p=0;p<ACTIVITY_CONFIG_LIST[activity].param_count;p++){
        if (ACTIVITY_CONFIG_LIST[activity].params[p].p_id == param_id) {
            ptr = &ACTIVITY_CONFIG_LIST[activity].params[p];
        }
    }

    if (activity_god_mode) {
        for (int p = 0;p < ACTIVITY_CONFIG_GOD.param_count;p++) {
            if (ACTIVITY_CONFIG_GOD.params[p].p_id == param_id) {
                ptr = &ACTIVITY_CONFIG_GOD.params[p];
            }
        }
    }

    return ptr;

}

int
init_activity_config_list(){

    if (activity_config_list_inited) return true;

    ACTIVITY_CONFIG_BASE = ActivityConfig(
        999,
        "Base",
        {
            //           Param name           Type         Device  Istart Iend  Step  Values
            ParamConfig("A_SAH_NO_SPOT",     VALUES_STR,    DOG,   -1,    -1,  -1,    {0,1}              ),
            ParamConfig("NAV_AFOL_MODE",     VALUES_STR,    DOG,   -1,    -1,  -1,    {0,1,2,3,4}        ),
            ParamConfig("OFF_FR_ADD_ANG",     VALUES_STR,    DOG,   -1,    -1,  -1,    {0,1,2,3,4,5,6,7}  ),
            ParamConfig("CBP_MAX_INIT_SPD",   INTERVAL,      DOG,   1,     10,  1,     {}                 ),
            ParamConfig("NAV_TAKEOFF_ALT",    INTERVAL,      DOG,   5,     50,  1,     {}                 ),
            ParamConfig("A_INIT_POS_D",       INTERVAL,      DOG,   5,     100, 1,     {}                 ),
            ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,    DOG,   -1,    -1,  -1,    {1,2}              ),
            ParamConfig("A_FOL_IMDTLY",       VALUES_STR,    DOG,   -1,    -1,  -1,    {0,1}              ),
            ParamConfig("RTL_RET_ALT",        INTERVAL,      DOG,   10,    100, 5,     {}                 ),
            ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,   -1,    -1,  -1,    {0,1}              ),
            ParamConfig("FOL_VEL_FF_XY",      INTERVAL,      DOG,    0,    2,   0.05f, {}                 ),
            ParamConfig("FOL_LPF_XY",         INTERVAL,      DOG,    0,    2,   0.05f, {}                 ),
            ParamConfig("A_INIT_POS_U",       VALUES_STR,    DOG,   -1,   -1,  -1,     {0,1}              ),
            ParamConfig("V_REACTION",         VALUES_STR,    DOG,   -1,   -1,  -1,     {0,1,2}            ),
            ParamConfig("SENS_SON_MIN",       INTERVAL,      DOG,    3,    20,  0.5f,  {}                 ),
            ParamConfig("AIRD_TRAJ_RAD",      INTERVAL,      ALL,    3,    20,  1,     {}                 ),
        });


    ACTIVITY_CONFIG_GOD = ActivityConfig(
        1000,
        "God",
        {
            //           Param name           Type         Device  Default Istart Iend  Step  Values
            
            ParamConfig("AIRD_TRAJ_RAD",      INTERVAL,     ALL,   3,    20,  1,     {}                 ),
            ParamConfig("FOL_FF_GRAD_END",    INTERVAL,     DOG,   10,   200, 5,     {}                 ),
            ParamConfig("FOL_FF_GRAD_STRT",   INTERVAL,     DOG,    0,    150, 5,     {}                 ),
            ParamConfig("FOL_FF_GRAD_USE",    VALUES_STR,   DOG,   -1,   -1,  -1,    {0,1}              ),
            ParamConfig("FOL_USE_ALT",        VALUES_STR,   DOG,   -1,   -1,  -1,    {0,1}              ), 
            ParamConfig("INAV_INIT_EPH",      INTERVAL,     DOG,    1,    5,   0.1f,  {}                 ),
            ParamConfig("INAV_INIT_EPV",      INTERVAL,     DOG,    1,    10,  0.1f,  {}                 ),
            ParamConfig("INAV_INIT_WAIT",     INTERVAL,     DOG,    4000, 300000, 1000,   {}             ),
            ParamConfig("MPC_PITCH_LPF",      INTERVAL,     DOG,    0,    500, 5,     {}                 ),
            ParamConfig("MPC_XY_FF",          INTERVAL,     DOG,    0,    1,   0.05f, {}                 ),
            ParamConfig("MPC_XY_P",           INTERVAL,     DOG,    0,    1,   0.05f, {}                 ),
            ParamConfig("MPC_XY_VEL_D",       INTERVAL_HP,  DOG,    0,    1,   0.0001f,  {}              ),
            ParamConfig("MPC_XY_VEL_I",       INTERVAL_HP,  DOG,    0,    1,   0.0001f,  {}              ),
            ParamConfig("MPC_XY_VEL_MAX",     INTERVAL,     DOG,    5,    25,  1,     {}                 ),
            ParamConfig("MPC_XY_VEL_P",       INTERVAL,     DOG,    0,    6,   0.05f, {}                 ),
            ParamConfig("MPC_Z_FF",           INTERVAL,     DOG,    0,    1,   0.05f, {}                 ),
            ParamConfig("MPC_Z_P",            INTERVAL,     DOG,    0,    6,   0.05f, {}                 ),
            ParamConfig("MPC_Z_VEL_D",        INTERVAL,     DOG,    0,    5,   0.01f, {}                 ),
            ParamConfig("MPC_Z_VEL_I",        INTERVAL,     DOG,    0,    5,   0.01f, {}                 ),
            ParamConfig("MPC_Z_VEL_MAX",      INTERVAL,     DOG,    1,    6,   0.5f,  {}                 ),
            ParamConfig("MPC_Z_VEL_P",        INTERVAL,     DOG,    0,    5,   0.01f, {}                 ),
            ParamConfig("PAFOL_GT_AC_DST",    INTERVAL,     DOG,    1,    10,  1,     {}                 ),
            ParamConfig("SDLOG_ON_BOOT",      VALUES_STR,   DOG,   -1,    -1,  -1,    {0,1}              ),
            ParamConfig("SENS_SON_ON",        VALUES_STR,   DOG,   -1,    -1,  -1,    {0,1}              ),
            ParamConfig("UBX_ENABLE_SBAS",    VALUES_STR,   DOG,   -1,    -1,  -1,    {0,1}              ),
            ParamConfig("UBX_GPS_MAX_CHN",    INTERVAL,     DOG,   8,    24,  1,     {}                 ),
            ParamConfig("UBX_GPS_MIN_CHN",    INTERVAL,     DOG,   1,    16,  1,     {}                 ),
            ParamConfig("INAV_W_Z_GPS_P",     INTERVAL_HP,  DOG,   0,    1,   0.001f,{}                 ),
            ParamConfig("INAV_W_Z_BARO",      INTERVAL,     DOG,   0,    1,   0.05f, {}                 ),
            ParamConfig("BAT_WARN_LVL",       INTERVAL,     DOG,   0,    1,   0.05,  {}                 ),
       });

    ACTIVITY_CONFIG_LIST[0] = ActivityConfig(
            ACTIVITY_TEST,
            activityNames[ACTIVITY_TEST],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {0,1,3,4}        ),
            });

    ACTIVITY_CONFIG_LIST[1] = ActivityConfig(
            ACTIVITY_SURF,
            activityNames[ACTIVITY_SURF],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("A_SAH_NO_SPOT",      INTERVAL,     DOG,      -1,    -1,  -1,    {1}              ),
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {0,3}            ),
                ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,    DOG,      -1,    -1,  -1,    {1}              ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,      -1,    -1,  -1,    {0}              ),
            });

    ACTIVITY_CONFIG_LIST[2] = ActivityConfig(
            ACTIVITY_SKI,
            activityNames[ACTIVITY_SKI],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {1,3}              ),
                ParamConfig("OFF_FR_ADD_ANG",     VALUES_STR,    DOG,      -1,    -1,  -1,    {0,4}              ),
                ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,    DOG,      -1,    -1,  -1,    {2}                ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,      -1,    -1,  -1,    {1}                ),
            });

    ACTIVITY_CONFIG_LIST[3] = ActivityConfig(
            ACTIVITY_SKATE,
            activityNames[ACTIVITY_SKATE],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {0}                ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,      -1,    -1,  -1,    {0}                ),
            });

    ACTIVITY_CONFIG_LIST[4] = ActivityConfig(
            ACTIVITY_CYCLE,
            activityNames[ACTIVITY_CYCLE],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {0,1,3}            ),
                ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,    DOG,      -1,    -1,  -1,    {2}                ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,      -1,    -1,  -1,    {1}                ),
            });

    ACTIVITY_CONFIG_LIST[5] = ActivityConfig(
            ACTIVITY_WAKE,
            activityNames[ACTIVITY_WAKE],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("A_SAH_NO_SPOT",      INTERVAL,     DOG,      -1,    -1,  -1,    {1}                ),
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {1,3}              ),
                ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,    DOG,      -1,    -1,  -1,    {1}                ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,      -1,    -1,  -1,    {0}                ),
            });

    ACTIVITY_CONFIG_LIST[6] = ActivityConfig(
            ACTIVITY_BIKE,
            activityNames[ACTIVITY_BIKE],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {1,3}             ),
                ParamConfig("OFF_FR_ADD_ANG",     VALUES_STR,    DOG,      -1,    -1,  -1,    {0,1,2,3,4,5,6,7} ),
            });

    ACTIVITY_CONFIG_LIST[7] = ActivityConfig(
            ACTIVITY_SNOWBOARD,
            activityNames[ACTIVITY_SNOWBOARD],
            {
                //           Param name           Type         Device  Default Istart Iend  Step  Values
                ParamConfig("A_SAH_NO_SPOT",      INTERVAL,     DOG,      -1,    -1,  -1,    {1}                ),
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {2}                ),
                ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,    DOG,      -1,    -1,  -1,    {1}                ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,      -1,    -1,  -1,    {0}                ),
            });

    ACTIVITY_CONFIG_LIST[8] = ActivityConfig(
            ACTIVITY_KITE,
            activityNames[ACTIVITY_KITE],
            {
                ParamConfig("A_SAH_NO_SPOT",      INTERVAL,      DOG,      -1,    -1,  -1,    {1}                ),
                ParamConfig("NAV_AFOL_MODE",      VALUES_STR,    DOG,      -1,    -1,  -1,    {0}                ),
                ParamConfig("A_BSC_SAF_ACT",      VALUES_STR,    DOG,      -1,    -1,  -1,    {1}                ),
                ParamConfig("FOL_RPT_ALT",        VALUES_STR,    DOG,      -1,    -1,  -1,    {0}                ),
            }
            );

    ACTIVITY_CONFIG_LIST[9] = ActivityConfig(
            ACTIVITY_CUSTOM,
            activityNames[ACTIVITY_CUSTOM],
            {
            });

    activity_config_list_inited = true;
    return true;


    };

    float DEFAULT_VALUES[ACTIVITIES_COUNT][ALLOWED_PARAM_COUNT] = {
    {0,0,1,0,5,10,12,1,0,20,0,1,1.2,0,2,6,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {1,1,3,0,5,7,12,1,1,20,0,1,1.5,0,2,5,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {2,0,1,4,5,10,12,2,0,35,1,1,1.5,0,2,7,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {3,0,0,0,5,10,12,1,0,20,0,1,1,0,2,3,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {4,0,1,4,5,10,10,2,0,30,1,1,1.5,0,2,6,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {5,1,3,0,5,10,12,1,1,20,0,1,1.5,0,2,5,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {6,1,1,4,5,12,12,1,1,35,1,1,1.5,0,2,7,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {7,1,1,0,5,10,8,1,1,20,0,1,1.2,0,2,4,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {8,1,0,0,5,8,15,1,0,40,0,1,1.7,0,2,5,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15},
    {9,0,1,0,5,8,10,1,0,20,0,1,1.5,0,2,5,5,150,20,1,1,2.5,3.5,15000,50,1,0.5,0.0007,0.0015,20,0.1,0.3,1.1,0.01,0.05,4,0.28,2,1,1,1,16,8,0.001,0.5,12,0,0,12,0,0,0.15}
    };

}
//End of namepsace Activity
