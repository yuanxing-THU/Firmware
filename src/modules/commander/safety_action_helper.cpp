#include <nuttx/config.h>

#include <quick_log/quick_log.hpp>

#include "safety_action_helper.hpp"

Safety_action_helper::Safety_action_helper()
    : do_param()
    , boot_init_complete(false)
    , do_checks_enabled(false)
    , land_allowed(false)
    , rth_allowed(false)
    , control_allowed_after_emergency(false)
{ }

commander_error_code Safety_action_helper::Boot_init() {
    boot_init_complete = false;
    do_checks_enabled = false;
    
    if ( !do_param.Open                    ("A_SAH_DO")      ) return SAH_ERROR;
    if ( !allowed_safety_actions_param.Open("A_SAH_ALLOWED") ) return SAH_ERROR;
    
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
    
    const int32_t allowed_safety_actions = allowed_safety_actions_param.Get();
    
    if ( (allowed_safety_actions & ~int32_t(Safety_action::Known_bits)) != 0 ) {
        QLOG_sprintf("[Safety_action_helper] weird bits on allowed actions: %u", allowed_safety_actions);
        return SAH_ERROR;
    }
    
    land_allowed = bool(allowed_safety_actions & int32_t(Safety_action::Land_on_spot));
    rth_allowed  = bool(allowed_safety_actions & int32_t(Safety_action::Return_to_home));
    
    if ( !land_allowed && !rth_allowed ) {
        QLOG_literal("[Safety_action_helper] all safety actions not allowed in activity");
        return SAH_ERROR;
    }
    
    control_allowed_after_emergency = bool(allowed_safety_actions & int32_t(Safety_action::Allow_control_after_emergency));
    
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
    allowed_safety_actions_param.Close();
}

Safety_action_helper::~Safety_action_helper() { }
