#ifndef __COMMANDER_SAFETY_ACTION_HELPER_HPP_INCLUDED__
#define __COMMANDER_SAFETY_ACTION_HELPER_HPP_INCLUDED__

#include <commander/commander_error.h>

#include <utils/param_reader.hpp>

class Safety_action_helper {
public:
    enum class Safety_action {
          Return_to_home = 1
        , Land_on_spot   = 2
    };
    
public:
    // Initializes the class, but does not open any files / uorbs / params / etc.
    Safety_action_helper();
    
    // To be called once after boot (and after Shutdown), opens any required files / uorbs / params / etc.
    commander_error_code Boot_init();
    
    // To be called right before takeoff.
    commander_error_code Takeoff_init();
    
    bool Allowed_to_land() const;
    bool Allowed_to_rth()  const;
    
    bool Control_allowed_after_emergency() const;
    
    void Shutdown();
    
    ~Safety_action_helper();
    
private:
    Utils::Param_reader<int32_t> do_param;
    Utils::Param_reader<int32_t> land_disallowed_param;
    Utils::Param_reader<int32_t> rth_allowed_param;
    Utils::Param_reader<int32_t> control_allowed_after_emergency_param;
    
private:
    bool boot_init_complete;
    bool do_checks_enabled;
    
private:
    bool land_allowed;
    bool rth_allowed;
    
    bool control_allowed_after_emergency;
    
private:
    Safety_action_helper(const Safety_action_helper &);
    Safety_action_helper & operator=(const Safety_action_helper &);
};

#endif
