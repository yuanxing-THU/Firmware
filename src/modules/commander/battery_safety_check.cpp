#include <nuttx/config.h>

#include "battery_safety_check.hpp"

#include <drivers/drv_hrt.h>
#include <quick_log/quick_log.hpp>
#include <math.h>

extern Safety_action_helper g_safety_action_helper;

Battery_safety_check::Battery_safety_check()
    : do_param()
    , safety_action_param()
    , check_interval_ms_param()
    , block_interval_ms_param()
    , rtl_ret_alt_m_param()
    , battery_capacity_mah_param()
    , battery_flat_lvl_param()
    , battery_land_lo_lvl_param()
    , battery_land_mi_lvl_param()
    , battery_land_hi_lvl_param()
    , battery_rth_lo_lvl_param()
    , battery_rth_mi_lvl_param()
    , battery_rth_hi_lvl_param()
    , battery_100m_gain_mah_param()
    , battery_100m_loss_mah_param()
    , battery_100m_dist_mah_param()
    , battery_rth_coef_param()
    , battery_lpf_hz_param()
    , boot_init_complete(false)
    , do_checks_enabled(false)
    , safety_action(Safety_action::Return_to_home)
    , user_notified(false)
    , action_taken(false)
    , death_done(false)
    , check_interval_ms(1)
    , block_interval_ms(1)
    , next_check_ms(0)
    , block_until_ms(0)
    , rtl_ret_alt_m(0.0f)
    , battery_capacity_mah(1.0f)
    , battery_flat_mah(100000.0f)
    , battery_land_lo_mah(0.0f)
    , battery_land_mi_mah(0.0f)
    , battery_land_hi_mah(0.0f)
    , battery_rth_lo_mah(0.0f)
    , battery_rth_mi_mah(0.0f)
    , battery_rth_hi_mah(0.0f)
    , battery_100m_gain_mah(100000.0f)
    , battery_100m_loss_mah(100000.0f)
    , battery_100m_dist_mah(100000.0f)
    , battery_rth_coef(1000000.0f)
    , home_mpr()
    , battery_level_lpf(0.0f)
    , filtered_battery_level(0.0f)
{ }

commander_error_code Battery_safety_check::Boot_init() {
    boot_init_complete = false;
    do_checks_enabled = false;
    
    if ( !do_param.Open                     ("A_BSC_DO")         ) return BSC_ERROR;
    if ( !safety_action_param.Open          ("A_BSC_SAF_ACT")    ) return BSC_ERROR;
    if ( !check_interval_ms_param.Open      ("A_BSC_INT_MS")     ) return BSC_ERROR;
    if ( !block_interval_ms_param.Open      ("A_BSC_BLOCK_MS")   ) return BSC_ERROR;
    if ( !rtl_ret_alt_m_param.Open          ("RTL_RET_ALT")      ) return BSC_ERROR;
    if ( !battery_capacity_mah_param.Open   ("BAT_CAPACITY")     ) return BSC_ERROR;
    if ( !battery_flat_lvl_param.Open       ("BAT_FLAT_LVL")     ) return BSC_ERROR;
    if ( !battery_land_lo_lvl_param.Open    ("A_BSC_LLO_LVL")    ) return BSC_ERROR;
    if ( !battery_land_mi_lvl_param.Open    ("A_BSC_LMI_LVL")    ) return BSC_ERROR;
    if ( !battery_land_hi_lvl_param.Open    ("A_BSC_LHI_LVL")    ) return BSC_ERROR;
    if ( !battery_rth_lo_lvl_param.Open     ("A_BSC_RLO_LVL")    ) return BSC_ERROR;
    if ( !battery_rth_mi_lvl_param.Open     ("A_BSC_RMI_LVL")    ) return BSC_ERROR;
    if ( !battery_rth_hi_lvl_param.Open     ("A_BSC_RHI_LVL")    ) return BSC_ERROR;
    if ( !battery_100m_gain_mah_param.Open  ("A_BSC_GAIN_MAH")   ) return BSC_ERROR;
    if ( !battery_100m_loss_mah_param.Open  ("A_BSC_LOSS_MAH")   ) return BSC_ERROR;
    if ( !battery_100m_dist_mah_param.Open  ("A_BSC_DIST_MAH")   ) return BSC_ERROR;
    if ( !battery_rth_coef_param.Open       ("A_BSC_RTH_COEF")   ) return BSC_ERROR;
    if ( !battery_lpf_hz_param.Open         ("A_BSC_BAT_HZ")     ) return BSC_ERROR;
    
    boot_init_complete = true;
    
    return COMMANDER_ERROR_OK;
}

bool Battery_safety_check::Param_lvl_to_mah(float & value, Utils::Param_reader<float> & param, const float l_min, const float l_max) {
    if ( battery_capacity_mah < BSC_CAP_MAH_MIN || battery_capacity_mah > BSC_CAP_MAH_MAX ) {
        QLOG_literal("[Battery_safety_check] insane capacity?");
        return false;
    }
    const bool res = Utils::Get_param(value, param, l_min, l_max);
    value *= battery_capacity_mah;
    return res;
}

commander_error_code Battery_safety_check::Takeoff_init(const home_position_s & home_pos) {
    do_checks_enabled = false;
    
    if ( !boot_init_complete ) {
        QLOG_literal("[Battery_safety_check] boot init not complete");
        return BSC_ERROR;
    }
    
    const bool temp_do_checks_enabled = (do_param.Get() != 0);
    if ( !temp_do_checks_enabled ) return COMMANDER_ERROR_OK;
    
    if ( home_pos.lat == 0.0 && home_pos.lon == 0.0 ) {
        QLOG_literal("[Battery_safety_check] Takeoff_init - home_pos looks uninitialized ...");
        return COMMANDER_ERROR_OK;
    }
    map_projection_init(&home_mpr, home_pos.lat, home_pos.lon);
    
    safety_action = Safety_action(safety_action_param.Get());
    if ( safety_action != Safety_action::Land_on_spot && safety_action != Safety_action::Return_to_home ) {
        QLOG_sprintf("[Battery_safety_check] bad safety action (%d)", int(safety_action));
        return BSC_ERROR;
    }
    
    if ( safety_action == Safety_action::Land_on_spot && !g_safety_action_helper.Allowed_to_land() ) {
        QLOG_literal("[Battery_safety_check] land on spot selected but not allowed");
        return BSC_ERROR;
    }
    if ( safety_action == Safety_action::Return_to_home && !g_safety_action_helper.Allowed_to_rth() ) {
        QLOG_literal("[Battery_safety_check] return to home selected but not allowed");
        return BSC_ERROR;
    }
    
    user_notified = false;
    action_taken = false;
    death_done = false;
    
    if ( !Utils::Get_param(check_interval_ms,       check_interval_ms_param,       BSC_INT_MS_MIN,   BSC_INT_MS_MAX)   ) return BSC_ERROR;
    if ( !Utils::Get_param(block_interval_ms,       block_interval_ms_param,       BSC_BLOCK_MS_MIN, BSC_BLOCK_MS_MAX) ) return BSC_ERROR;
    next_check_ms = 0;  // Check as soon as needed / possible, schedule next interval-timed check then.
    block_until_ms = 0; // Nothing blocks any actions initially.
    
    rtl_ret_alt_m = rtl_ret_alt_m_param.Get();
    
    if ( !Utils::Get_param(battery_capacity_mah,    battery_capacity_mah_param,    BSC_CAP_MAH_MIN,  BSC_CAP_MAH_MAX)  ) return BSC_ERROR;
    
    if ( !Param_lvl_to_mah(battery_flat_mah,        battery_flat_lvl_param,        BSC_FLAT_LVL_MIN, BSC_FLAT_LVL_MAX) ) return BSC_ERROR;
    
    if ( !Param_lvl_to_mah(battery_land_lo_mah,     battery_land_lo_lvl_param,     BSC_LLO_LVL_MIN,  BSC_LLO_LVL_MAX)  ) return BSC_ERROR;
    if ( !Param_lvl_to_mah(battery_land_mi_mah,     battery_land_mi_lvl_param,     BSC_LMI_LVL_MIN,  BSC_LMI_LVL_MAX)  ) return BSC_ERROR;
    if ( !Param_lvl_to_mah(battery_land_hi_mah,     battery_land_hi_lvl_param,     BSC_LHI_LVL_MIN,  BSC_LHI_LVL_MAX)  ) return BSC_ERROR;
    
    if ( !Param_lvl_to_mah(battery_rth_lo_mah,      battery_rth_lo_lvl_param,      BSC_RLO_LVL_MIN,  BSC_RLO_LVL_MAX)  ) return BSC_ERROR;
    if ( !Param_lvl_to_mah(battery_rth_mi_mah,      battery_rth_mi_lvl_param,      BSC_RMI_LVL_MIN,  BSC_RMI_LVL_MAX)  ) return BSC_ERROR;
    if ( !Param_lvl_to_mah(battery_rth_hi_mah,      battery_rth_hi_lvl_param,      BSC_RHI_LVL_MIN,  BSC_RHI_LVL_MAX)  ) return BSC_ERROR;
    
    if ( !Utils::Get_param(battery_100m_gain_mah,   battery_100m_gain_mah_param,   BSC_GAIN_MAH_MIN, BSC_GAIN_MAH_MAX) ) return BSC_ERROR;
    if ( !Utils::Get_param(battery_100m_loss_mah,   battery_100m_loss_mah_param,   BSC_LOSS_MAH_MIN, BSC_LOSS_MAH_MAX) ) return BSC_ERROR;
    if ( !Utils::Get_param(battery_100m_dist_mah,   battery_100m_dist_mah_param,   BSC_DIST_MAH_MIN, BSC_DIST_MAH_MAX) ) return BSC_ERROR;
    if ( !Utils::Get_param(battery_rth_coef,        battery_rth_coef_param,        BSC_RTH_COEF_MIN, BSC_RTH_COEF_MAX) ) return BSC_ERROR;
    
    float battery_lpf_hz = 0.0f;
    if ( !Utils::Get_param(battery_lpf_hz,          battery_lpf_hz_param,          BSC_BAT_HZ_MIN,   BSC_BAT_HZ_MAX)   ) return BSC_ERROR;
    battery_level_lpf.set_cutoff_frequency(battery_lpf_hz);
    filtered_battery_level = 1.0f; // Will be updated on first flight check.
    
    do_checks_enabled = true;
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Battery_safety_check::Flight_check(vehicle_status_s & status
        , const home_position_s & home_pos
        , const vehicle_global_position_s & veh_pos
        , const bool manual_control_enabled) const {
    if ( !do_checks_enabled ) return COMMANDER_ERROR_OK;
    
    if ( !status.condition_battery_voltage_valid ) {
        QLOG_literal("[Battery_safety_check] battery voltage not valid");
        return BSC_ERROR;
    }
    
    const uint64_t now_time_us = hrt_absolute_time();
    filtered_battery_level = battery_level_lpf.apply(now_time_us, status.battery_remaining);
    
    const uint64_t now_time_ms = now_time_us / 1000;
    
    if ( now_time_ms < next_check_ms ) {
        return COMMANDER_ERROR_OK;
    } else {
        next_check_ms = now_time_ms + check_interval_ms;
    }
    
    if ( safety_action == Safety_action::Land_on_spot ) {
        return Flight_land_on_spot_check(now_time_ms, status, manual_control_enabled);
    } else if ( safety_action == Safety_action::Return_to_home ) {
        return Flight_rth_check(now_time_ms, status, home_pos, veh_pos, manual_control_enabled);
    }
    
    QLOG_literal("[Battery_safety_check] bad safety action selected?");
    return BSC_ERROR;
}

bool Battery_safety_check::Should_land(const float battery_level) const {
    if ( !do_checks_enabled ) return false;
    const float remaining_mah = battery_level * battery_capacity_mah;
    return remaining_mah < (battery_flat_mah + battery_land_hi_mah);
}

bool Battery_safety_check::Must_land(const float battery_level) const {
    if ( !do_checks_enabled ) return false;
    const float remaining_mah = battery_level * battery_capacity_mah;
    return remaining_mah < (battery_flat_mah + battery_land_mi_mah);
}

bool Battery_safety_check::Death_land(const float battery_level) const {
    if ( !do_checks_enabled ) return false;
    const float remaining_mah = battery_level * battery_capacity_mah;
    return remaining_mah < (battery_flat_mah + battery_land_lo_mah);
}

float Battery_safety_check::Mah_to_rth(const home_position_s & home_pos, const vehicle_global_position_s & veh_pos) const {
    float x_m = 0.0f, y_m = 0.0f;
    
    map_projection_project(&home_mpr, veh_pos.lat, veh_pos.lon, &x_m, &y_m);
    const float distance_to_home_m    = sqrtf(x_m*x_m + y_m*y_m);
    
    const float safe_alt_to_home_m    = rtl_ret_alt_m;
    const float safe_alt_m            = home_pos.alt + safe_alt_to_home_m;
    const float veh_to_safe_alt_m     = safe_alt_m - veh_pos.alt;
    
    float total_mah_to_get_home = 0.0f;
    if ( veh_to_safe_alt_m >= 0.0f ) {
        total_mah_to_get_home +=  veh_to_safe_alt_m * battery_100m_gain_mah / 100.0f;
    } else {
        total_mah_to_get_home += -veh_to_safe_alt_m * battery_100m_loss_mah / 100.0f;
    }
    total_mah_to_get_home += distance_to_home_m * battery_100m_dist_mah / 100.0f;
    total_mah_to_get_home += safe_alt_to_home_m * battery_100m_loss_mah / 100.0f;
    total_mah_to_get_home *= battery_rth_coef;
    
    return total_mah_to_get_home;
}

bool Battery_safety_check::Should_rth(const float battery_level, const float mah_to_rth) const {
    if ( !do_checks_enabled ) return false;
    const float remaining_mah = battery_level * battery_capacity_mah;
    return (remaining_mah - mah_to_rth) < (battery_flat_mah + battery_rth_hi_mah);
}

bool Battery_safety_check::Must_rth(const float battery_level, const float mah_to_rth) const {
    if ( !do_checks_enabled ) return false;
    const float remaining_mah = battery_level * battery_capacity_mah;
    return (remaining_mah - mah_to_rth) < (battery_flat_mah + battery_rth_mi_mah);
}

bool Battery_safety_check::Can_rth(const float battery_level, const float mah_to_rth) const {
    if ( !do_checks_enabled ) return true;
    const float remaining_mah = battery_level * battery_capacity_mah;
    return (remaining_mah - mah_to_rth) > (battery_flat_mah + battery_rth_lo_mah);
}

commander_error_code Battery_safety_check::Flight_rth_check(const uint64_t now_time_ms
        , vehicle_status_s & status
        , const home_position_s & home_pos
        , const vehicle_global_position_s & veh_pos
        , const bool manual_control_enabled) const {
    const bool blocked = now_time_ms < block_until_ms;
    
    if ( action_taken && !manual_control_enabled && !blocked
            && status.main_state != MAIN_STATE_EMERGENCY_RTL
            && status.main_state != MAIN_STATE_EMERGENCY_LAND ) {
        user_notified = false;
        action_taken = false;
        death_done = false;
    }
    
    const float mah_to_rth = Mah_to_rth(home_pos, veh_pos);
    
    const bool should_rth = g_safety_action_helper.Allowed_to_rth()  && Should_rth(filtered_battery_level, mah_to_rth) && !blocked;
    const bool must_rth   = g_safety_action_helper.Allowed_to_rth()  && Must_rth  (filtered_battery_level, mah_to_rth) && !blocked;
    const bool can_rth    =                                             Can_rth   (filtered_battery_level, mah_to_rth);
    const bool death_land = g_safety_action_helper.Allowed_to_land() && Death_land(filtered_battery_level);
    
    if ( should_rth ) {
        status.battery_warning = VEHICLE_BATTERY_WARNING_LOW;
        if ( !user_notified && !must_rth && !death_land ) {
            user_notified = true;
            QLOG_literal("[Battery_safety_check] BSC_ERROR_BATTERY_RTH_NOTIFY");
            return BSC_ERROR_BATTERY_RTH_NOTIFY;
        }
    }
    
    if ( must_rth ) {
        status.battery_warning = VEHICLE_BATTERY_WARNING_CRITICAL;
        if ( !action_taken && !death_land ) {
            user_notified = true;
            action_taken = true;
            if ( can_rth ) {
                QLOG_literal("[Battery_safety_check] BSC_ERROR_BATTERY_RTH");
                return BSC_ERROR_BATTERY_RTH;
            } else {
                return Unexpected_battery_drop_below_can_rth_when_rth_selected(status);
            }
        }
    }
    
    if ( death_land ) {
        status.battery_warning = VEHICLE_BATTERY_WARNING_FLAT;
        if ( !death_done ) {
            user_notified = true;
            action_taken = true;
            death_done = true;
            QLOG_literal("[Battery_safety_check] BSC_ERROR_BATTERY_LAND_DEATH");
            return BSC_ERROR_BATTERY_LAND_DEATH;
        }
    }
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Battery_safety_check::Flight_land_on_spot_check(const uint64_t now_time_ms
        , vehicle_status_s & status, const bool manual_control_enabled) const {
    const bool blocked = now_time_ms < block_until_ms;
    
    if ( action_taken && !manual_control_enabled && !blocked && status.main_state != MAIN_STATE_EMERGENCY_LAND ) {
        user_notified = false;
        action_taken = false;
        death_done = false;
    }
    
    const bool should_land = g_safety_action_helper.Allowed_to_land() && Should_land(filtered_battery_level) && !blocked;
    const bool must_land   = g_safety_action_helper.Allowed_to_land() && Must_land  (filtered_battery_level) && !blocked;
    const bool death_land  = g_safety_action_helper.Allowed_to_land() && Death_land (filtered_battery_level);
    
    if ( should_land ) {
        status.battery_warning = VEHICLE_BATTERY_WARNING_LOW;
        if ( !user_notified && !must_land && !death_land ) {
            user_notified = true;
            QLOG_literal("[Battery_safety_check] BSC_ERROR_BATTERY_LAND_NOTIFY");
            return BSC_ERROR_BATTERY_LAND_NOTIFY;
        }
    }
    
    if ( must_land ) {
        status.battery_warning = VEHICLE_BATTERY_WARNING_CRITICAL;
        if ( !action_taken && !death_land ) {
            user_notified = true;
            action_taken = true;
            QLOG_literal("[Battery_safety_check] BSC_ERROR_BATTERY_LAND");
            return BSC_ERROR_BATTERY_LAND;
        }
    }
    
    if ( death_land ) {
        status.battery_warning = VEHICLE_BATTERY_WARNING_FLAT;
        if ( !death_done ) {
            user_notified = true;
            action_taken = true;
            death_done = true;
            QLOG_literal("[Battery_safety_check] BSC_ERROR_BATTERY_LAND_DEATH");
            return BSC_ERROR_BATTERY_LAND_DEATH;
        }
    }
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Battery_safety_check::Unexpected_battery_drop_below_can_rth_when_rth_selected(vehicle_status_s & status) const {
    if ( g_safety_action_helper.Allowed_to_land() ) {
        safety_action = Safety_action::Land_on_spot;
        user_notified = false;
        action_taken = false;
        death_done = false;
        status.battery_warning = VEHICLE_BATTERY_WARNING_LOW;
        QLOG_literal("[Battery_safety_check] BSC_ERROR_BATTERY_SWITCH_RTH_TO_LAND");
        return BSC_ERROR_BATTERY_SWITCH_RTH_TO_LAND;
    } else {
        QLOG_literal("[Battery_safety_check] BSC_ERROR_BATTERY_RTH_WO_BATT");
        return BSC_ERROR_BATTERY_RTH_WO_BATT;
    }
}

void Battery_safety_check::On_user_action() const {
    if ( action_taken ) {
        const uint64_t now_time_ms = hrt_absolute_time() / 1000;
        block_until_ms = now_time_ms + block_interval_ms;
    }
}

void Battery_safety_check::Shutdown() {
    boot_init_complete = false;
    do_checks_enabled = false;
    
    do_param.Close();
    safety_action_param.Close();
    check_interval_ms_param.Close();
    block_interval_ms_param.Close();
    rtl_ret_alt_m_param.Close();
    battery_capacity_mah_param.Close();
    battery_flat_lvl_param.Close();
    battery_land_lo_lvl_param.Close();
    battery_land_mi_lvl_param.Close();
    battery_land_hi_lvl_param.Close();
    battery_rth_lo_lvl_param.Close();
    battery_rth_mi_lvl_param.Close();
    battery_rth_hi_lvl_param.Close();
    battery_100m_gain_mah_param.Close();
    battery_100m_loss_mah_param.Close();
    battery_100m_dist_mah_param.Close();
    battery_rth_coef_param.Close();
    battery_lpf_hz_param.Close();
}

Battery_safety_check::~Battery_safety_check() { }
