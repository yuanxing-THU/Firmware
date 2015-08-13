#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum commander_error_code
{
    COMMANDER_ERROR_OK                     = 0,
    COMMANDER_ERROR_NOT_ACTIVATED          = 1,
    
    PMC_ERROR                              = 10,
    PMC_ERROR_INITIAL_TILT                 = 11,
    PMC_ERROR_TOO_MUCH_VIBRATION           = 12,
    PMC_ERROR_TOO_MUCH_TILT                = 13,
    
    FTC_ERROR                              = 20,
    FTC_ERROR_TAKEOFF_TOO_MUCH_TILT        = 21,
    FTC_ERROR_TAKEOFF_NO_ALTITUDE_GAIN     = 22,
    FTC_ERROR_FLIGHT_TOO_MUCH_TILT         = 23,
    FTC_ERROR_LANDING_TOO_MUCH_TILT        = 24,
};

__EXPORT int commander_set_error(int error_code);

#ifdef __cplusplus
}
#endif
