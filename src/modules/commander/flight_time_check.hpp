#ifndef __COMMANDER_FLIGHT_TIME_CHECK_HPP_INCLUDED__
#define __COMMANDER_FLIGHT_TIME_CHECK_HPP_INCLUDED__

#include <commander/commander_error.h>

#include <utils/param_reader.hpp>
#include <utils/orb_subscriber_w_data.hpp>

#include <uORB/topics/vehicle_attitude.h>
#include <uORB/topics/vehicle_local_position.h>

#define FTC_TTILT_DEG_MIN      1
#define FTC_TTILT_DEG_MAX      60

#define FTC_TALT_M_MIN         1.0f
#define FTC_TALT_M_MAX         10.0f

#define FTC_TALT_MS_MIN        1000
#define FTC_TALT_MS_MAX        20000

#define FTC_FTILT_DEG_MIN      1
#define FTC_FTILT_DEG_MAX      180

#define FTC_FLTILT_DEG_MIN     1
#define FTC_FLTILT_DEG_MAX     180

#define FTC_FLTILT_MS_MIN      1000
#define FTC_FLTILT_MS_MAX      20000

#define FTC_LTILT_DEG_MIN      1
#define FTC_LTILT_DEG_MAX      90

class Flight_time_check {
public:
    // Initializes the class, but does not open any files / uorbs / params / etc.
    Flight_time_check();
    
    // To be called once after boot (and after Shutdown), opens any required files / uorbs / params / etc.
    commander_error_code Boot_init();
    
    // To be called right before takeoff from land.
    commander_error_code Takeoff_init();
    
    // To be called if a repeat in-air takeoff takes place.
    void On_in_air_takeoff();
    
    // Performs any checks required during takeoff.
    commander_error_code Takeoff_check() const;
    
    // Performs any checks required during flight.
    commander_error_code Flight_check()  const;
    
    // Performs any checks required during landing.
    commander_error_code Landing_check() const;
    
    void Shutdown();
    
    ~Flight_time_check();
    
private:
    commander_error_code Verify_tilt_not_exceeded(const float min_tilt_cos, commander_error_code error_code) const;
    
private:
    Utils::Param_reader<int32_t> do_param;
    Utils::Param_reader<int32_t> takeoff_tilt_deg_param;
    Utils::Param_reader<float  > takeoff_alt_m_param;
    Utils::Param_reader<int32_t> takeoff_alt_ms_param;
    Utils::Param_reader<int32_t> flight_tilt_deg_param;
    Utils::Param_reader<int32_t> flight_low_tilt_deg_param;
    Utils::Param_reader<int32_t> flight_low_tilt_ms_param;
    Utils::Param_reader<int32_t> landing_tilt_deg_param;
    
private:
    Utils::ORB_subscriber_w_data<ORB_ID(vehicle_attitude),       vehicle_attitude_s>       attitude_orb;
    Utils::ORB_subscriber_w_data<ORB_ID(vehicle_local_position), vehicle_local_position_s> local_pos_orb;
    
private:
    bool boot_init_complete;
    bool do_checks_enabled;
    
private:
    float takeoff_tilt_min_cos;
    
    float takeoff_need_z_diff;
    uint64_t takeoff_time_to_achieve_z_diff_ms;
    
    float takeoff_initial_z;
    uint64_t takeoff_start_time_ms;
    mutable float takeoff_max_seen_z_diff;
    
    float flight_tilt_min_cos;
    float flight_low_tilt_min_cos;
    float flight_low_tilt_timeout_ms;
    
    // 0 if low tilt limit was not exceeded on last tick, otherwise the time when the limit was first noticed over the limit
    mutable uint64_t flight_low_tilt_exceeded_start_time_ms;
    
    float landing_tilt_min_cos;
    
private:
    Flight_time_check(const Flight_time_check &);
    Flight_time_check & operator=(const Flight_time_check &);
};

#endif
