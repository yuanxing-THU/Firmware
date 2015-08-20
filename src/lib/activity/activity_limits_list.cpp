#include <stdio.h>
#include <string.h>

#include "activity_limits_list.hpp"

namespace Activity {

bool activity_limits_list_inited = false;
ActivityLimits ACTIVITY_LIMITS_LIST[ ACTIVITIES_COUNT+1 ];

ParamLimits::ParamLimits (){
    p_id = -1;
    type = -1;
    value_num = 0;
};

ParamLimits::ParamLimits(const char * _name, int _type, int _target_device, float _istart, float _iend, float _step, std::initializer_list <float> _values) {

    p_id = -1;

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++)
        if (strcmp(ALLOWED_PARAMS[i].name, _name) == 0 && _target_device == ALLOWED_PARAMS[i].target_device)
            p_id = ALLOWED_PARAMS[i].id;

    if (p_id == -1) {

        // TODO: change to proper debug output function 
        printf("Parameter: %s with target device type %d not allowed.\n", _name, _target_device); 
        fflush(stdout);
    }

    type = _type;
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

ParamLimits::~ParamLimits(){
    delete [] values;
};

ParamLimits& 
ParamLimits::operator = (const ParamLimits &a) {

    p_id = a.p_id;
    type = a.type;

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

ParamLimits::ParamLimits(const ParamLimits &a) {
    *this = a;
}

const char * 
ParamLimits::get_name(){
    return ALLOWED_PARAMS[p_id].name;
};

param_target_device 
ParamLimits::get_target_device(){
    return ALLOWED_PARAMS[p_id].target_device;
}

const char * 
ParamLimits::get_param_name(){
    return ALLOWED_PARAMS[p_id].name;
}

const char * 
ParamLimits::get_display_name(){
    return ALLOWED_PARAMS[p_id].display_name;
}

ActivityLimits::ActivityLimits(){
    id = -1;
    name = nullptr;
    param_count = 0;
    params = nullptr;
};

ActivityLimits::ActivityLimits(int _param_count){
    param_count = _param_count;
    params = new ParamLimits[param_count];
}

ActivityLimits::ActivityLimits(int _id, const char * _name, std::initializer_list <ParamLimits> _params ){

    id = _id;
    name = _name;
 
    param_count = _params.size();
    params = new ParamLimits[param_count];

    int i = 0;
    for (auto p : _params) {
        params[i] = p;
        ++i;
    }
}

ActivityLimits::~ActivityLimits() {
   delete [] params;
}


ActivityLimits& 
ActivityLimits::operator=(ActivityLimits &&a) {

    id = a.id;
    param_count = a.param_count;
    name = a.name;

    delete [] params;

    params = a.params;
    a.params = nullptr;
    
}

int 
init_activity_limits_list(){

    ACTIVITY_LIMITS_LIST[0] = ActivityLimits(
            0, 
            "DEFAULT_LIMITS", 
            {
            //    Param name            Type        Device  Istart Iend  Step  Values  
            ParamLimits("A_ACTIVITY",         INVISIBLE,  ALL,    -1,    -1,   -1,   {}    ),
            ParamLimits("A_BT_CONNECT_TO",    INVISIBLE,  DOG,     2,    -1,   -1,   {}    ),
            ParamLimits("NAV_DLL_OBC",      INVISIBLE,  DOG,    -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_BW_DST_LIM",   INTERVAL,   DOG,     6,    12,   2,    {}    ),
            ParamLimits("PAFOL_VPID_I",       INVISIBLE,  DOG,    -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_P",       INVISIBLE,  DOG,    -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_D",       INVISIBLE,  DOG,    -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_OPT_D",        INTERVAL,   DOG,    12,    30,   1,    {}    ),
            ParamLimits("NAV_TAKEOFF_ALT",    INTERVAL,   DOG,    10,    20,   2,    {}    ),
            ParamLimits("NAV_AFOL_MODE",      VALUES_STR, DOG,    -1,    -1,   -1,   {0,1,2} ),
            ParamLimits("FOL_VEL_FF_XY",         INTERVAL,   DOG,    0.1f,  2.0f, 0.2f, {}    ),
            ParamLimits("FOL_USE_ALT",        VALUES_INT, DOG,    -1,    -1,   -1,   {0,1} ),
            ParamLimits("FOL_RPT_ALT",        VALUES_INT, DOG,    -1,    -1,   -1,   {0,1} ),
            ParamLimits("FOL_VEL_FF_Z",        VALUES_INT, DOG,    -1,    -1,   -1,   {0,1} ),
            ParamLimits("MPC_XY_VEL_P",       INVISIBLE,  DOG,    -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_I",       INVISIBLE,  DOG,    -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_D",       INVISIBLE,  DOG,    -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_P",       INVISIBLE,  LEASH,  -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_I",       INVISIBLE,  LEASH,  -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_D",       INVISIBLE,  LEASH,  -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_BUFF_SIZE",       INVISIBLE,  DOG,  0,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_FF",          INVISIBLE,  DOG,    -1,    -1,   -1,   {}    ),
            });

    ACTIVITY_LIMITS_LIST[1] = ActivityLimits( 1, 
            "KITEBOARDING", 
            {
            //    Param name            Type        Device   Istart Iend  Step  Values  
            ParamLimits("A_ACTIVITY",         INVISIBLE,  ALL,     -1,    -1,   -1,   {}    ), 
            ParamLimits("MPC_XY_VEL_D",       INVISIBLE,  LEASH,  -1,     -1,   -1,   {2.0f, 3.0f} ),
            ParamLimits("PAFOL_OPT_D",        VALUES_INT,   DOG,     12,    30,   1,    {10,15,30}    ),
            ParamLimits("FOL_VEL_FF_XY",         VALUES_FLOAT,   DOG,     0.1f,  2.0f, 0.2f, {1.1f, 2.2f, 3.5f} ),
            });

    ACTIVITY_LIMITS_LIST[2] = ActivityLimits(
            2, 
            "SKATEBOARDING", 
            {
            //    Param name            Type        Device   Istart Iend  Step  Values  
            ParamLimits("A_ACTIVITY",         INVISIBLE,  ALL,     -1,    -1,   -1,   {}    ), 
            ParamLimits("PAFOL_VPID_I",       INVISIBLE,  DOG,     0,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_P",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_D",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_OPT_D",        INTERVAL,   DOG,     12,    30,   1,    {}    ),
            ParamLimits("FOL_VEL_FF_XY",         INTERVAL,   DOG,     0.1f,  2.0f, 0.2f, {}    ),
            ParamLimits("FOL_USE_ALT",        VALUES_STR, DOG,     -1,   -1,    -1,   {0,1} ),
            ParamLimits("FOL_RPT_ALT",        VALUES_STR, DOG,     -1,   -1,    -1,   {0,1} ),
            ParamLimits("MPC_XY_VEL_P",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_I",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_D",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_FF",          INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            });

    ACTIVITY_LIMITS_LIST[3] = ActivityLimits(
            3, 
            "CHESS", 
            {
            //    Param name            Type        Device   Istart Iend  Step  Values  
            ParamLimits("A_ACTIVITY",         INVISIBLE,  ALL,     1,    -1,   -1,   {}    ), 
            ParamLimits("A_BT_CONNECT_TO",    INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_BW_DST_LIM",   INTERVAL,   DOG,      6,    12,   2,    {}    ),
            ParamLimits("PAFOL_VPID_I",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_P",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_D",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_OPT_D",        INTERVAL,   DOG,     12,    30,   1,    {}    ),
            ParamLimits("FOL_USE_ALT",        VALUES_STR, DOG,     -1,   -1,    -1,   {0,1} ),
            ParamLimits("FOL_RPT_ALT",        VALUES_STR, DOG,     -1,   -1,    -1,   {0,1} ),
            });

    ACTIVITY_LIMITS_LIST[4] = ActivityLimits(
            4, 
            "SLALOM", 
            {
            //    Param name            Type        Device   Istart Iend  Step  Values  
            ParamLimits("A_ACTIVITY",         INVISIBLE,  ALL,     -1,    -1,   -1,   {}    ), 
            ParamLimits("PAFOL_VPID_I",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_P",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_D",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_OPT_D",        INTERVAL,   DOG,     12,    30,   1,    {}    ),
            ParamLimits("MPC_XY_VEL_P",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_I",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_D",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ), 
            });

    ACTIVITY_LIMITS_LIST[5] = ActivityLimits(
            5, 
            "CROSSFIT", 
            {
            //    Param name            Type        Istart Iend  Step  Values  
            ParamLimits("A_ACTIVITY",         INVISIBLE,  ALL,     -1,    -1,   -1,   {}    ), 
            ParamLimits("PAFOL_VPID_I",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_P",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("PAFOL_VPID_D",       INTERVAL,  DOG,     5.0f,    10.0f,   1.0f,   {}    ),
            ParamLimits("FOL_USE_ALT",        VALUES_INT, DOG,     -1,   -1,    -1,   {0,1} ),
            ParamLimits("FOL_RPT_ALT",        VALUES_STR, DOG,     -1,   -1,    -1,   {0,1} ),
            ParamLimits("MPC_XY_VEL_P",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_I",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_D",       INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_VEL_D",       INVISIBLE,  LEASH,   -1,    -1,   -1,   {}    ),
            ParamLimits("MPC_XY_FF",          INVISIBLE,  DOG,     -1,    -1,   -1,   {}    ),
            });
};

}
//End of namepsace Activity
