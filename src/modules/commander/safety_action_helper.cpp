#include <nuttx/config.h>

#include <quick_log/quick_log.hpp>

#include "safety_action_helper.hpp"

Safety_action_helper::Safety_action_helper()
    : do_param()
    , land_disallowed_param()
    , rth_allowed_param()
    , control_allowed_after_emergency_param()
    , do_checks_enabled(false)
    , land_allowed(false)
    , rth_allowed(false)
    , control_allowed_after_emergency(false)
{ }

commander_error_code Safety_action_helper::Boot_init() {
    boot_init_complete = false;
    do_checks_enabled = false;
    
    if ( !do_param.Open                             ("A_SAH_DO")      ) return SAH_ERROR;
    if ( !land_disallowed_param.Open                ("A_SAH_NO_SPOT") ) return SAH_ERROR;
    if ( !rth_allowed_param.Open                    ("A_SAH_RTH")     ) return SAH_ERROR;
    if ( !control_allowed_after_emergency_param.Open("A_SAH_CTRL")    ) return SAH_ERROR;
    
    boot_init_complete = true;
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Safety_action_helper::Takeoff_init() {
    do_checks_enabled = false;
    
    if ( !boot_init_complete ) {
        QLOG_literal("[Safety_action_helper] boot init not complete");
        return SAH_ERROR;
    }
    
    const bool temp_do_checks_enabled = (do_param.Get() != 0);
    if ( !temp_do_checks_enabled ) return COMMANDER_ERROR_OK;
    
    land_allowed                    = !bool(land_disallowed_param.Get());
    rth_allowed                     =  bool(rth_allowed_param.Get());
    control_allowed_after_emergency =  bool(control_allowed_after_emergency_param.Get());
    
    if ( !land_allowed && !rth_allowed ) {
        QLOG_literal("[Safety_action_helper] all safety actions not allowed in activity");
        return SAH_ERROR;
    }
    
    do_checks_enabled = true;
    
    return COMMANDER_ERROR_OK;
}

bool Safety_action_helper::Allowed_to_land() const {
    return !do_checks_enabled || land_allowed;
}

bool Safety_action_helper::Allowed_to_rth() const {
    return !do_checks_enabled || rth_allowed;
}

bool Safety_action_helper::Control_allowed_after_emergency() const {
    return !do_checks_enabled || control_allowed_after_emergency;
}

void Safety_action_helper::Shutdown() {
    boot_init_complete = false;
    do_checks_enabled = false;
    
    do_param.Close();
    land_disallowed_param.Close();
    rth_allowed_param.Close();
    control_allowed_after_emergency_param.Close();
}

Safety_action_helper::~Safety_action_helper() { }
