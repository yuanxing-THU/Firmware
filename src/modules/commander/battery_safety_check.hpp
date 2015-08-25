#ifndef __COMMANDER_BATTERY_SAFETY_CHECK_HPP_INCLUDED__
#define __COMMANDER_BATTERY_SAFETY_CHECK_HPP_INCLUDED__

#include <commander/commander_error.h>

#include <geo/geo.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter.hpp>

#include <uORB/topics/home_position.h>
#include <uORB/topics/vehicle_global_position.h>
#include <uORB/topics/vehicle_status.h>

#include <utils/param_reader.hpp>

#include "safety_action_helper.hpp"

#define BSC_INT_MS_MIN         1
#define BSC_INT_MS_MAX         5000

#define BSC_CAP_MAH_MIN        1000
#define BSC_CAP_MAH_MAX        20000

#define BSC_FLAT_LVL_MIN       0.00f
#define BSC_FLAT_LVL_MAX       0.50f

#define BSC_RLO_LVL_MIN        0.00f
#define BSC_RLO_LVL_MAX        0.50f

#define BSC_RMI_LVL_MIN        0.00f
#define BSC_RMI_LVL_MAX        0.50f

#define BSC_RHI_LVL_MIN        0.00f
#define BSC_RHI_LVL_MAX        0.50f

#define BSC_LLO_LVL_MIN        0.00f
#define BSC_LLO_LVL_MAX        0.50f

#define BSC_LMI_LVL_MIN        0.00f
#define BSC_LMI_LVL_MAX        0.50f

#define BSC_LHI_LVL_MIN        0.00f
#define BSC_LHI_LVL_MAX        0.50f

#define BSC_GAIN_MAH_MIN       10.0f
#define BSC_GAIN_MAH_MAX       1000.0f

#define BSC_LOSS_MAH_MIN       10.0f
#define BSC_LOSS_MAH_MAX       1000.0f

#define BSC_DIST_MAH_MIN       10.0f
#define BSC_DIST_MAH_MAX       1000.0f

#define BSC_RTH_COEF_MIN       1.00f
#define BSC_RTH_COEF_MAX       2.00f

#define BSC_BAT_HZ_MIN         0.00f
#define BSC_BAT_HZ_MAX         20.0f

class Battery_safety_check {
public:
    typedef Safety_action_helper::Safety_action Safety_action;
    
public:
    // Initializes the class, but does not open any files / uorbs / params / etc.
    Battery_safety_check();
    
    // To be called once after boot (and after Shutdown), opens any required files / uorbs / params / etc.
    commander_error_code Boot_init();
    
    // To be called right before takeoff.
    commander_error_code Takeoff_init(const home_position_s & home_pos);
    
    // Performs any checks required during flight. We will update battery_warning in vehicle_status_s, if needed.
    commander_error_code Flight_check(vehicle_status_s & status
        , const home_position_s & home_pos, const vehicle_global_position_s & veh_pos
        , const bool manual_control_enabled) const;
    
    // These return true if drone should/must/MUST land, based on battery level alone, does not check whether the action is allowed.
    bool Should_land(const float battery_level) const;
    bool Must_land  (const float battery_level) const;
    bool Death_land (const float battery_level) const;
    
    float Mah_to_rth(const home_position_s & home_pos, const vehicle_global_position_s & veh_pos) const;
    
    // These return true if drone should/must/can rth, based on battery level alone, does not check whether the action is allowed.
    bool Should_rth (const float battery_level, const float mah_to_rth) const;
    bool Must_rth   (const float battery_level, const float mah_to_rth) const;
    bool Can_rth    (const float battery_level, const float mah_to_rth) const;
    
    void Shutdown();
    
    ~Battery_safety_check();
    
private:
    commander_error_code Flight_rth_check(const uint64_t now_time_ms, vehicle_status_s & status
        , const home_position_s & home_pos, const vehicle_global_position_s & veh_pos
        , const bool manual_control_enabled) const;
    
    commander_error_code Flight_land_on_spot_check(const uint64_t now_time_ms, vehicle_status_s & status
        , const bool manual_control_enabled) const;
    
    commander_error_code Unexpected_battery_drop_below_can_rth_when_rth_selected(vehicle_status_s & status) const;
    
    bool Param_lvl_to_mah(float & value, Utils::Param_reader<float> & param, const float l_min, const float l_max);
    
private:
    Utils::Param_reader<int32_t> do_param;
    Utils::Param_reader<int32_t> safety_action_param;
    Utils::Param_reader<int32_t> check_interval_ms_param;
    Utils::Param_reader<float  > rtl_ret_alt_m_param;
    Utils::Param_reader<float  > battery_capacity_mah_param;
    Utils::Param_reader<float  > battery_flat_lvl_param;
    Utils::Param_reader<float  > battery_land_lo_lvl_param;
    Utils::Param_reader<float  > battery_land_mi_lvl_param;
    Utils::Param_reader<float  > battery_land_hi_lvl_param;
    Utils::Param_reader<float  > battery_rth_lo_lvl_param;
    Utils::Param_reader<float  > battery_rth_mi_lvl_param;
    Utils::Param_reader<float  > battery_rth_hi_lvl_param;
    Utils::Param_reader<float  > battery_100m_gain_mah_param;
    Utils::Param_reader<float  > battery_100m_loss_mah_param;
    Utils::Param_reader<float  > battery_100m_dist_mah_param;
    Utils::Param_reader<float  > battery_rth_coef_param;
    Utils::Param_reader<float  > battery_lpf_hz_param;
    
private:
    bool boot_init_complete;
    bool do_checks_enabled;
    
private:
    mutable Safety_action safety_action;
    mutable bool user_notified;
    mutable bool action_taken;
    mutable bool death_done;
    
    uint64_t check_interval_ms;
    mutable uint64_t next_check_ms;
    
    float rtl_ret_alt_m;
    float battery_capacity_mah;
    float battery_flat_mah;
    
    float battery_land_lo_mah;
    float battery_land_mi_mah;
    float battery_land_hi_mah;
    
    float battery_rth_lo_mah;
    float battery_rth_mi_mah;
    float battery_rth_hi_mah;
    
    float battery_100m_gain_mah;
    float battery_100m_loss_mah;
    float battery_100m_dist_mah;
    float battery_rth_coef;
    
    struct map_projection_reference_s home_mpr;
    mutable math::LowPassFilter<float> battery_level_lpf;
    mutable float filtered_battery_level;
    
private:
    Battery_safety_check(const Battery_safety_check &);
    Battery_safety_check & operator=(const Battery_safety_check &);
};

#endif
