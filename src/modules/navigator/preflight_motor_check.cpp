#include <nuttx/config.h>

#include "preflight_motor_check.hpp"

#include <drivers/drv_hrt.h>
#include <geo/geo.h>
#include <poll.h>
#include <math.h>

#include "navigator.h"

const math::Vector<3> Preflight_motor_check::s_up_vector(0.0f, 0.0f, 1.0f);

// Initialize stuff to some "safe values", these will not be used in the actual tests,
// they will be set to the real values in Setup, from environment params, etc.
Preflight_motor_check::Preflight_motor_check(Navigator * navigatorp)
    : navigator(navigatorp)
    , do_param()
    , servo_pwm()
    , boot_init_complete(false)
    , do_checks_enabled(false)
    , vibr_threshold_m_s2(0.0f)
    , tilt_change_min_cos(2.0f)
    , ramp_ms(0)
    , hold_ms(0)
    , timer_precision_ms(1)
    , thrust_percent(0)
    , initial_up_vector(0.0f, 0.0f, -1.0f)
    , min_pwm()
    , max_pwm()
    // , accelerometer_samples()
    // , last_accelerometer_sample_timestamp(0)
{
    min_pwm.channel_count = 0;
    max_pwm.channel_count = 0;
    for ( int i = 0; i < PWM_OUTPUT_MAX_CHANNELS; ++i ) {
        min_pwm.values[i] = 0;
        max_pwm.values[i] = 0;
    }
}

commander_error_code Preflight_motor_check::Boot_init() {
    boot_init_complete = false;
    do_checks_enabled = false;
    
    if ( !do_param.Open("A_PMC_DO") )              return PMC_ERROR;
    if ( !servo_pwm.Open(PWM_OUTPUT_DEVICE_PATH) ) return PMC_ERROR;
    
    boot_init_complete = true;
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Preflight_motor_check::Pre_check_init() {
    do_checks_enabled = false;
    
    if ( !boot_init_complete ) {
        printf("[Preflight_motor_check] Pre_check_init - boot init not complete\n");
        return PMC_ERROR;
    }
    
    const bool temp_do_checks_enabled = (do_param.Get() != 0);
    if ( !temp_do_checks_enabled ) return COMMANDER_ERROR_OK;
    
    int32_t itilt_deg = 0, ctilt_deg = 0;
    if ( !Utils::Get_param<int32_t>(itilt_deg,           "A_PMC_ITILT_DEG",  PMC_ITILT_DEG_MIN,  PMC_ITILT_DEG_MAX)  ) return PMC_ERROR;
    if ( !Utils::Get_param<int32_t>(ctilt_deg,           "A_PMC_CTILT_DEG",  PMC_CTILT_DEG_MIN,  PMC_CTILT_DEG_MAX)  ) return PMC_ERROR;
    if ( !Utils::Get_param<int32_t>(ramp_ms,             "A_PMC_RAMP_MS",    PMC_RAMP_MS_MIN,    PMC_RAMP_MS_MAX)    ) return PMC_ERROR;
    if ( !Utils::Get_param<int32_t>(hold_ms,             "A_PMC_HOLD_MS",    PMC_HOLD_MS_MIN,    PMC_HOLD_MS_MAX)    ) return PMC_ERROR;
    if ( !Utils::Get_param<int32_t>(timer_precision_ms,  "A_PMC_PREC_MS",    PMC_PREC_MS_MIN,    PMC_PREC_MS_MAX)    ) return PMC_ERROR;
    if ( !Utils::Get_param<int32_t>(thrust_percent,      "A_PMC_THRUST_PCT", PMC_THRUST_PCT_MIN, PMC_THRUST_PCT_MAX) ) return PMC_ERROR;
    if ( !Utils::Get_param<float  >(vibr_threshold_m_s2, "A_PMC_VIBR_THRSH", PMC_VIBR_THRSH_MIN, PMC_VIBR_THRSH_MAX) ) return PMC_ERROR;
    
    {
        const float initial_tilt_max_rad = math::radians(float(itilt_deg));
        const float tilt_change_max_rad  = math::radians(float(ctilt_deg));
        const float initial_tilt_min_cos = cos(initial_tilt_max_rad);
        tilt_change_min_cos              = cos(tilt_change_max_rad);
        
        navigator->public_vehicle_attitude_update();
        vehicle_attitude_s * vehicle_attitude = navigator->get_vehicle_attitude();
        if ( !vehicle_attitude->R_valid ) {
            printf("[Preflight_motor_check] Pre_check_init - vehicle_attitude not R_valid\n");
            return PMC_ERROR;
        }
        
        math::Matrix<3,3> drone_to_world_matrix(vehicle_attitude->R);
        initial_up_vector = drone_to_world_matrix * s_up_vector;
        const float initial_tilt_cos = s_up_vector * initial_up_vector;
        if ( initial_tilt_cos < initial_tilt_min_cos ) {
            printf("[Preflight_motor_check] Pre_check_init - initial tilt too big: cos = %.4f\n", double(initial_tilt_cos));
            return PMC_ERROR_INITIAL_TILT;
        }
    }
    
    {
        unsigned reported_servo_count = 0;
        if ( !servo_pwm.IOctl(PWM_SERVO_GET_COUNT, (unsigned long) &reported_servo_count) ) return PMC_ERROR;
        if ( reported_servo_count < PMC_USE_SERVO_COUNT ) {
            printf("[Preflight_motor_check] Pre_check_init - bad reported_servo_count: %u\n", reported_servo_count);
            return PMC_ERROR;
        }
        if ( !servo_pwm.IOctl(PWM_SERVO_GET_MIN_PWM, (unsigned long) &min_pwm) ) return PMC_ERROR;
        if ( !servo_pwm.IOctl(PWM_SERVO_GET_MAX_PWM, (unsigned long) &max_pwm) ) return PMC_ERROR;
    }
    
    // Uncomment this if we ever want to start doing some history based accelerometer magic, not just on the last sample.
    // accelerometer_samples.Clear();
    // last_accelerometer_sample_timestamp = 0;
    
    do_checks_enabled = true;
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Preflight_motor_check::Check() {
    if ( !do_checks_enabled ) return COMMANDER_ERROR_OK;
    
    printf("[Preflight_motor_check] Check - started\n");
    
    const commander_error_code execute_error_code = Execute();
    if ( execute_error_code != COMMANDER_ERROR_OK ) {
        // Make sure we try to set all motors to safe values before reporting failure.
        int min_pwm_set_fails = 0;
        for ( unsigned servo = 0; servo < PMC_USE_SERVO_COUNT; ++servo ) {
            if ( !servo_pwm.IOctl(PWM_SERVO_SET(servo), min_pwm.values[servo], false) ) ++min_pwm_set_fails;
        }
        if ( min_pwm_set_fails != 0 ) {
            printf("[Preflight_motor_check] Do - min_pwm_set_fails: %d\n", min_pwm_set_fails);
        }
        
        if ( execute_error_code == PMC_ERROR_TOO_MUCH_VIBRATION ) {
            printf("[Preflight_motor_check] Check - failed: too much vibration\n");
        } else if ( execute_error_code == PMC_ERROR_TOO_MUCH_TILT ) {
            printf("[Preflight_motor_check] Check - failed: too much tilt\n");
        } else {
            printf("[Preflight_motor_check] Check - failed: unknown failure\n");
        }
        return execute_error_code;
    }
    
    printf("[Preflight_motor_check] Check - OK\n");
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Preflight_motor_check::Execute() const {
    const commander_error_code ramp_phase_error_code = Run_ramp_phase();
    if ( ramp_phase_error_code != COMMANDER_ERROR_OK ) return ramp_phase_error_code;
    
    const commander_error_code hold_phase_error_code = Run_hold_phase();
    if ( hold_phase_error_code != COMMANDER_ERROR_OK ) return hold_phase_error_code;
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Preflight_motor_check::Run_ramp_phase() const {
    const uint64_t start_time_ms = hrt_absolute_time() / 1000; // us -> ms
    const uint64_t end_time_ms   = start_time_ms + ramp_ms;
    
    for ( uint64_t now_time_ms = start_time_ms; now_time_ms < end_time_ms; now_time_ms = hrt_absolute_time() / 1000 ) {
        const unsigned time_percent = unsigned((now_time_ms - start_time_ms) * 100 / (end_time_ms - start_time_ms));
        const unsigned use_thrust_percent = thrust_percent * time_percent / 100;
        
        for ( unsigned servo = 0; servo < PMC_USE_SERVO_COUNT; ++servo ) {
            const unsigned pwm = min_pwm.values[servo] + unsigned(max_pwm.values[servo]-min_pwm.values[servo])*use_thrust_percent/100;
            if ( !servo_pwm.IOctl(PWM_SERVO_SET(servo), pwm, false) ) return PMC_ERROR;
        }
        
        const commander_error_code poll_and_check_error_code = Poll_and_check();
        if ( poll_and_check_error_code != COMMANDER_ERROR_OK ) return poll_and_check_error_code;
    }
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Preflight_motor_check::Run_hold_phase() const {
    const uint64_t start_time_ms = hrt_absolute_time() / 1000; // us -> ms
    const uint64_t end_time_ms   = start_time_ms + hold_ms;
    
    for ( uint64_t now_time_ms = start_time_ms; now_time_ms < end_time_ms; now_time_ms = hrt_absolute_time() / 1000 ) {
        for ( unsigned servo = 0; servo < PMC_USE_SERVO_COUNT; ++servo ) {
            const unsigned pwm = min_pwm.values[servo] + unsigned(max_pwm.values[servo]-min_pwm.values[servo])*thrust_percent/100;
            if ( !servo_pwm.IOctl(PWM_SERVO_SET(servo), pwm, false) ) return PMC_ERROR;
        }
        
        const commander_error_code poll_and_check_error_code = Poll_and_check();
        if ( poll_and_check_error_code != COMMANDER_ERROR_OK ) return poll_and_check_error_code;
    }
    
    return COMMANDER_ERROR_OK;
}

commander_error_code Preflight_motor_check::Poll_and_check() const {
    const int poll_ret = navigator->public_poll_update_sensor_combined_and_vehicle_attitude(timer_precision_ms);
    if ( poll_ret < 0 ) {
        return PMC_ERROR;
    } else if ( poll_ret > 0 ) {
        {
            sensor_combined_s * sensor_combined = navigator->get_sensor_combined();
            const Sample sample(sensor_combined->accelerometer_m_s2[0]
                              , sensor_combined->accelerometer_m_s2[1]
                              , sensor_combined->accelerometer_m_s2[2]);
            const float threshold = vibr_threshold_m_s2;
            const float acc_diff = CONSTANTS_ONE_G * CONSTANTS_ONE_G + threshold * threshold;
            const float to_compare = 2.0f * CONSTANTS_ONE_G * threshold;
            const float tot_acc = sample.x*sample.x + sample.y*sample.y + sample.z*sample.z;
            if ( fabsf(tot_acc - acc_diff) > to_compare ) return PMC_ERROR_TOO_MUCH_VIBRATION;
            //Uncomment this if we ever want to start doing some history based accelerometer magic, not just on the last sample.
            //if ( last_accelerometer_sample_timestamp != sensor_combined->accelerometer_timestamp ) {
            //    last_accelerometer_sample_timestamp = sensor_combined->accelerometer_timestamp;
            //    accelerometer_samples.Push(Sample(sensor_combined->accelerometer_m_s2[0]
            //                                    , sensor_combined->accelerometer_m_s2[1]
            //                                    , sensor_combined->accelerometer_m_s2[2]));
            //    ...
            //}
        }
        {
            vehicle_attitude_s * vehicle_attitude = navigator->get_vehicle_attitude();
            if ( !vehicle_attitude->R_valid ) return PMC_ERROR;
            const math::Matrix<3,3> drone_to_world_matrix(vehicle_attitude->R);
            const math::Vector<3> current_up_vector = drone_to_world_matrix * s_up_vector;
            const float current_tilt_change_cos = current_up_vector * initial_up_vector;
            if ( current_tilt_change_cos < tilt_change_min_cos ) return PMC_ERROR_TOO_MUCH_TILT;
        }
    }
    
    return COMMANDER_ERROR_OK;
}

Preflight_motor_check::~Preflight_motor_check() { }
