#include <nuttx/config.h>

#include "flight_time_check.hpp"

#include <drivers/drv_hrt.h>
#include <lib/mathlib/mathlib.h>
#include <math.h>

Flight_time_check::Flight_time_check()
    : do_param()
    , takeoff_tilt_deg_param()
    , takeoff_alt_m_param()
    , takeoff_alt_ms_param()
    , flight_tilt_deg_param()
    , flight_low_tilt_deg_param()
    , flight_low_tilt_ms_param()
    , landing_tilt_deg_param()
    , attitude_orb()
    , local_pos_orb()
    , boot_init_complete(false)
    , do_checks_enabled(false)
    , takeoff_tilt_min_cos(2.0f)
    , takeoff_need_z_diff(10000.0f)
    , takeoff_time_to_achieve_z_diff_ms(0)
    , takeoff_initial_z(0.0f)
    , takeoff_start_time_ms(0)
    , takeoff_max_seen_z_diff(0.0f)
    , flight_tilt_min_cos(2.0f)
    , flight_low_tilt_min_cos(2.0f)
    , flight_low_tilt_timeout_ms(0)
    , flight_low_tilt_exceeded_start_time_ms(0)
    , landing_tilt_min_cos(2.0f)
{ }

commander_error_code Flight_time_check::Boot_init() {
    boot_init_complete = false;
    do_checks_enabled = false;
    
    if ( !do_param.Open("A_FTC_DO")                          ) return FTC_ERROR;
    if ( !takeoff_tilt_deg_param.Open("A_FTC_TTILT_DEG")     ) return FTC_ERROR;
    if ( !takeoff_alt_m_param.Open("A_FTC_TALT_M")           ) return FTC_ERROR;
    if ( !takeoff_alt_ms_param.Open("A_FTC_TALT_MS")         ) return FTC_ERROR;
    if ( !flight_tilt_deg_param.Open("A_FTC_FTILT_DEG")      ) return FTC_ERROR;
    if ( !flight_low_tilt_deg_param.Open("A_FTC_FLTILT_DEG") ) return FTC_ERROR;
    if ( !flight_low_tilt_ms_param.Open("A_FTC_FLTILT_MS")   ) return FTC_ERROR;
    if ( !landing_tilt_deg_param.Open("A_FTC_LTILT_DEG")     ) return FTC_ERROR;
    if ( !attitude_orb.Open()                                ) return FTC_ERROR;
    if ( !local_pos_orb.Open()                               ) return FTC_ERROR;
    
    boot_init_complete = true;
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Flight_time_check::Takeoff_init() {
    do_checks_enabled = false;
    
    if ( !boot_init_complete ) {
        printf("[Flight_time_check] Takeoff_init - boot init not complete\n");
        return FTC_ERROR;
    }
    
    const bool temp_do_checks_enabled = (do_param.Get() != 0);
    if ( !temp_do_checks_enabled ) return COMMANDER_ERROR_OK;
    
    {
        int32_t takeoff_tilt_deg = 0;
        if ( !Utils::Get_param(takeoff_tilt_deg, takeoff_tilt_deg_param, FTC_TTILT_DEG_MIN, FTC_TTILT_DEG_MAX) ) return FTC_ERROR;
        const float takeoff_tilt_max_rad = math::radians(float(takeoff_tilt_deg));
        takeoff_tilt_min_cos             = cos(takeoff_tilt_max_rad);
    }
    
    if ( !Utils::Get_param(takeoff_need_z_diff,               takeoff_alt_m_param,  FTC_TALT_M_MIN,  FTC_TALT_M_MAX)  ) return FTC_ERROR;
    if ( !Utils::Get_param(takeoff_time_to_achieve_z_diff_ms, takeoff_alt_ms_param, FTC_TALT_MS_MIN, FTC_TALT_MS_MAX) ) return FTC_ERROR;
    
    {
        if ( !local_pos_orb.Read() ) return FTC_ERROR;
        
        if ( !local_pos_orb.Data().z_valid ) {
            printf("[Flight_time_check] Takeoff_init - vehicle_local_position not z_valid\n");
            return FTC_ERROR;
        }
        
        takeoff_initial_z       = local_pos_orb.Data().z;
        takeoff_start_time_ms   = hrt_absolute_time() / 1000;
        takeoff_max_seen_z_diff = 0.0f;
    }
    
    {
        int32_t flight_tilt_deg = 0;
        if ( !Utils::Get_param(flight_tilt_deg, flight_tilt_deg_param, FTC_FTILT_DEG_MIN, FTC_FTILT_DEG_MAX) ) return FTC_ERROR;
        const float flight_tilt_max_rad = math::radians(float(flight_tilt_deg));
        flight_tilt_min_cos             = cos(flight_tilt_max_rad);
    }
    
    {
        int32_t flight_low_tilt_deg = 0;
        if ( !Utils::Get_param(flight_low_tilt_deg, flight_low_tilt_deg_param, FTC_FLTILT_DEG_MIN, FTC_FLTILT_DEG_MAX) ) return FTC_ERROR;
        const float flight_low_tilt_max_rad = math::radians(float(flight_low_tilt_deg));
        flight_low_tilt_min_cos             = cos(flight_low_tilt_max_rad);
    }
    
    if ( !Utils::Get_param(flight_low_tilt_timeout_ms, flight_low_tilt_ms_param, FTC_FLTILT_MS_MIN, FTC_FLTILT_MS_MAX) ) return FTC_ERROR;
    flight_low_tilt_exceeded_start_time_ms = 0;
    
    {
        int32_t landing_tilt_deg = 0;
        if ( !Utils::Get_param(landing_tilt_deg, landing_tilt_deg_param, FTC_LTILT_DEG_MIN, FTC_LTILT_DEG_MAX) ) return FTC_ERROR;
        const float landing_tilt_max_rad = math::radians(float(landing_tilt_deg));
        landing_tilt_min_cos             = cos(landing_tilt_max_rad);
    }
    
    do_checks_enabled = true;
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Flight_time_check::Takeoff_check() const {
    if ( !do_checks_enabled ) return COMMANDER_ERROR_OK;
    
    const commander_error_code tilt_verification_code = Verify_tilt_not_exceeded(takeoff_tilt_min_cos, FTC_ERROR_TAKEOFF_TOO_MUCH_TILT);
    if ( tilt_verification_code != COMMANDER_ERROR_OK ) return tilt_verification_code;
    
    bool local_pos_updated = false;
    if ( !local_pos_orb.Check(&local_pos_updated, false) ) return FTC_ERROR;
    
    if ( local_pos_updated ) {
        if ( !local_pos_orb.Read(false) )    return FTC_ERROR;
        if ( !local_pos_orb.Data().z_valid ) return FTC_ERROR;
        
        const float diff_z = fabsf(local_pos_orb.Data().z - takeoff_initial_z);
        if ( diff_z > takeoff_max_seen_z_diff ) takeoff_max_seen_z_diff = diff_z;
    }
    
    const uint64_t now_time_ms = hrt_absolute_time() / 1000;
    if ( now_time_ms - takeoff_start_time_ms > takeoff_time_to_achieve_z_diff_ms ) {
        if ( takeoff_max_seen_z_diff < takeoff_need_z_diff ) {
            return FTC_ERROR_TAKEOFF_NO_ALTITUDE_GAIN;
        }
    }
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Flight_time_check::Flight_check() const {
    if ( !do_checks_enabled ) return COMMANDER_ERROR_OK;
    
    const commander_error_code low_tilt_error_code = Verify_tilt_not_exceeded(flight_low_tilt_min_cos, FTC_ERROR_FLIGHT_TOO_MUCH_TILT);
    if ( low_tilt_error_code == FTC_ERROR_FLIGHT_TOO_MUCH_TILT ) {
        const uint64_t now_time_ms = hrt_absolute_time() / 1000;
        if ( flight_low_tilt_exceeded_start_time_ms == 0 ) {
            flight_low_tilt_exceeded_start_time_ms = now_time_ms;
        } else {
            if ( now_time_ms - flight_low_tilt_exceeded_start_time_ms > flight_low_tilt_timeout_ms ) {
                return FTC_ERROR_FLIGHT_TOO_MUCH_TILT;
            }
        }
    } else if ( low_tilt_error_code == COMMANDER_ERROR_OK ) {
        flight_low_tilt_exceeded_start_time_ms = 0;
    } else {
        return low_tilt_error_code;
    }
    
    return Verify_tilt_not_exceeded(flight_tilt_min_cos, FTC_ERROR_FLIGHT_TOO_MUCH_TILT);
}

commander_error_code Flight_time_check::Landing_check() const {
    if ( !do_checks_enabled ) return COMMANDER_ERROR_OK;
    return Verify_tilt_not_exceeded(landing_tilt_min_cos, FTC_ERROR_LANDING_TOO_MUCH_TILT);
}

commander_error_code Flight_time_check::Verify_tilt_not_exceeded(const float tilt_min_cos, commander_error_code error_code) const {
    bool attitude_updated = false;
    
    if ( !attitude_orb.Check(&attitude_updated, false) ) return FTC_ERROR;
    if ( attitude_updated ) {
        if ( !attitude_orb.Read() ) return FTC_ERROR;
    }
    
    if ( !attitude_orb.Data().R_valid ) return FTC_ERROR;
    
    const math::Vector<3> up_vector(0.0f, 0.0f, 1.0f);
    const math::Matrix<3,3> drone_to_world_matrix(attitude_orb.Data().R);
    const math::Vector<3> current_up_vector = drone_to_world_matrix * up_vector;
    const float current_tilt_cos = current_up_vector * up_vector;
    if ( current_tilt_cos < tilt_min_cos ) return error_code;
    
    return COMMANDER_ERROR_OK;
}

void Flight_time_check::Shutdown() {
    boot_init_complete = false;
    do_checks_enabled = false;
    
    do_param.Close();
    takeoff_tilt_deg_param.Close();
    takeoff_alt_m_param.Close();
    takeoff_alt_ms_param.Close();
    flight_tilt_deg_param.Close();
    landing_tilt_deg_param.Close();
    attitude_orb.Close();
    local_pos_orb.Close();
}

Flight_time_check::~Flight_time_check() { }
