#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * Enable preflight check. 0 = disable, any non-zero value = enable.
 */
PARAM_DEFINE_INT32(A_PMC_DO, 1);

/**
 * Configure, in degrees, the maximum tilt that a preflight check can be started at. Limited to [PMC_ITILT_DEG_MIN;PMC_ITILT_DEG_MAX].
 */
PARAM_DEFINE_INT32(A_PMC_ITILT_DEG, 30);

/**
 * Configure, in degrees, the maximum tilt change allowed during a preflight check. Limited to [PMC_CTILT_DEG_MIN;PMC_CTILT_DEG_MAX].
 */
PARAM_DEFINE_INT32(A_PMC_CTILT_DEG, 15);

/**
 * Configure, in milliseconds, the duration of the motor ramp-up phase. Limited to [PMC_RAMP_MS_MIN;PMC_RAMP_MS_MAX].
 */
PARAM_DEFINE_INT32(A_PMC_RAMP_MS, 750);

/**
 * Configure, in milliseconds, the duration of the motor hold phase. Limited to [PMC_HOLD_MS_MIN;PMC_HOLD_MS_MAX].
 */
PARAM_DEFINE_INT32(A_PMC_HOLD_MS, 750);

/**
 * Configure, in milliseconds, the precision of the preflight check timer. Limited to [PMC_PREC_MS_MIN;PMC_PREC_MS_MAX].
 */
PARAM_DEFINE_INT32(A_PMC_PREC_MS, 5);

/**
 * Configure, in percent, the maximum thrust reached during the preflight check. Limited to [PMC_THRUST_PCT_MIN;PMC_THRUST_PCT_MAX].
 */
PARAM_DEFINE_INT32(A_PMC_THRUST_PCT, 40);

/**
 * Configure, in m/s^2, the maximum acceleration allowed on top of 1G, during a check. Limited to [PMC_VIBR_THRSH_MIN;PMC_VIBR_THRSH_MAX].
 */
PARAM_DEFINE_FLOAT(A_PMC_VIBR_THRSH, 5.0f);
