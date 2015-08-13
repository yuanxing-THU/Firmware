#ifndef __NAVIGATOR_PREFLIGHT_MOTOR_CHECK_HPP_INCLUDED__
#define __NAVIGATOR_PREFLIGHT_MOTOR_CHECK_HPP_INCLUDED__

#include <commander/commander_error.h>
#include <drivers/drv_pwm_output.h>
#include <lib/mathlib/mathlib.h>
#include <utils/file_handle.hpp>
#include <utils/param_reader.hpp>

// How many servos of the ones available over PWM_OUTPUT_DEVICE_PATH are to be used by us. Starting from from index 0.
#define PMC_USE_SERVO_COUNT   4

// How many samples of accelerometer history to store.
// #define PMC_NUM_ACCEL_SAMPLES 5

#define PMC_ITILT_DEG_MIN     1
#define PMC_ITILT_DEG_MAX     45

#define PMC_CTILT_DEG_MIN     1
#define PMC_CTILT_DEG_MAX     45

#define PMC_RAMP_MS_MIN       300
#define PMC_RAMP_MS_MAX       5000

#define PMC_HOLD_MS_MIN       300
#define PMC_HOLD_MS_MAX       5000

#define PMC_PREC_MS_MIN       1
#define PMC_PREC_MS_MAX       50

#define PMC_THRUST_PCT_MIN    10
#define PMC_THRUST_PCT_MAX    50

#define PMC_VIBR_THRSH_MIN    0.1f
#define PMC_VIBR_THRSH_MAX    10.0f

class Navigator;

class Preflight_motor_check {
public:
    // Initializes the class, but does not open any files / uorbs / params / etc.
    Preflight_motor_check(Navigator * navigatorp);
    
    // To be called once after boot, opens any required files / uorbs / params / etc.
    commander_error_code Boot_init();
    
    // To be called right before the preflight motor check is performed.
    commander_error_code Pre_check_init();
    
    // Performs all the preflight motor check logic.
    commander_error_code Check();
    
    ~Preflight_motor_check();
    
private:
    commander_error_code Execute() const;
    
    commander_error_code Run_ramp_phase() const;
    commander_error_code Run_hold_phase() const;
    
    commander_error_code Poll_and_check() const;
    
private:
    Navigator * navigator;
    
private:
    Utils::Param_reader<int32_t> do_param;
    Utils::File_handle servo_pwm;
    
private:
    bool boot_init_complete;
    bool do_checks_enabled;
    
private:
    float vibr_threshold_m_s2;
    
    // Cosine value of our angle limit. If this falls _below_ the limit, that means the angle is _above_ the limit. 
    float tilt_change_min_cos;
    
    uint64_t ramp_ms;
    uint64_t hold_ms;
    unsigned timer_precision_ms;
    unsigned thrust_percent;
    
    // The up vector, in world space, our drone had, when the check was started (during Pre_check_init).
    math::Vector<3> initial_up_vector;
    
    pwm_output_values min_pwm, max_pwm;
    
private:
    // The agreed upon "up" vector in any space.
    static const math::Vector<3> s_up_vector;
    
private:
    // Struct for storing a single accelerometer reading sample.
    struct Sample {
        Sample(const float xp = 0.0f, const float yp = 0.0f, const float zp = 0.0f) : x(xp), y(yp), z(zp) { }
        Sample & operator=(const Sample & rhs) { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }
        float x, y, z;
    };
    
    //Uncomment this if we ever want to start doing some history based accelerometer magic, not just on the last sample.
    //// A circular buffer of up to MaxNumSamples.
    //// Starts at Length 0, and you can Push samples onto it.
    //// Use operator[] to get samples with indexes from 0 to the current Length.
    //// Sample [0] will be the oldest one, Sample [Length-1] will be the newest one.
    //template<int MaxNumSamples>
    //class Accelerometer_samples {
    //public:
    //    Accelerometer_samples() : start(0), length(0) { }
    //    int Length() const { return length; }
    //    
    //    void Clear() { length = 0; }
    //    
    //    // If you ask for an out-of-range index, you will get junk.
    //    const Sample & operator[](const int index) const {
    //        return samples[(start+index) % MaxNumSamples];
    //    }
    //    
    //    // Push a new Sample into the buffer, discarding the oldest one, if needed.
    //    const Sample & Push(const Sample & sample) {
    //        const int index = (start+length) % MaxNumSamples;
    //        samples[index] = sample;
    //        if ( length < MaxNumSamples ) {
    //            ++length;
    //        } else {
    //            start = (start+1) % MaxNumSamples;
    //        }
    //        return samples[index];
    //    }
    //    
    //    // Pop the oldest sample from the buffer, if any. You will get junk if Length was 0.
    //    Sample Pop() {
    //        const int old_start = start;
    //        if ( length > 0 ) {
    //            start = (start+1) % MaxNumSamples;
    //            --length;
    //        }
    //        return samples[old_start];
    //    }
    //    
    //private:
    //    int start, length;
    //    Sample samples[MaxNumSamples];
    //};
    //
    //mutable Accelerometer_samples<PMC_NUM_ACCEL_SAMPLES> accelerometer_samples;
    //mutable uint64_t last_accelerometer_sample_timestamp;
    
private:
    Preflight_motor_check(const Preflight_motor_check &);
    Preflight_motor_check & operator=(const Preflight_motor_check &);
};

#endif
