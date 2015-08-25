#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * Enable battery safety check. 0 = disable, any non-zero value = enable.
 */
PARAM_DEFINE_INT32(A_BSC_DO, 1);

/**
 * Configure the safety action to be taken for low battery levels. 1 = land on spot, 2 = return to home.
 */
PARAM_DEFINE_INT32(A_BSC_SAF_ACT, 2);

/**
 * Configure, in milliseconds, the interval between battery safety checks. Limited to [BSC_INT_MS_MIN;BSC_INT_MS_MAX].
 */
PARAM_DEFINE_INT32(A_BSC_INT_MS, 2000);

/**
 * Configure, in milliseconds, the interval that a user action blocks safety actions for. Limited to [BSC_BLOCK_MS_MIN;BSC_BLOCK_MS_MAX].
 */
PARAM_DEFINE_INT32(A_BSC_BLOCK_MS, 20000);

/**
 * Configure Rth Battery Low-Spare level. Limited to [BSC_RLO_LVL_MIN;BSC_RLO_LVL_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_RLO_LVL, 0.05f);

/**
 * Configure Rth Battery Mid-Spare level. Limited to [BSC_RMI_LVL_MIN;BSC_RMI_LVL_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_RMI_LVL, 0.15f);

/**
 * Configure Rth Battery High-Spare level. Limited to [BSC_RHI_LVL_MIN;BSC_RHI_LVL_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_RHI_LVL, 0.25f);

/**
 * Configure Land Battery Low-Spare level. Limited to [BSC_LLO_LVL_MIN;BSC_LLO_LVL_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_LLO_LVL, 0.03f);

/**
 * Configure Land Battery Mid-Spare level. Limited to [BSC_LMI_LVL_MIN;BSC_LMI_LVL_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_LMI_LVL, 0.06f);

/**
 * Configure Land Battery High-Spare level. Limited to [BSC_LHI_LVL_MIN;BSC_LHI_LVL_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_LHI_LVL, 0.16f);

/**
 * Configure estimated battery usage per 100m altitude gain. Limited to [BSC_GAIN_MAH_MIN;BSC_GAIN_MAH_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_GAIN_MAH, 160.0f);

/**
 * Configure estimated battery usage per 100m altitude loss. Limited to [BSC_LOSS_MAH_MIN;BSC_LOSS_MAH_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_LOSS_MAH, 220.0f);

/**
 * Configure estimated battery usage per 100m horizontal distance. Limited to [BSC_DIST_MAH_MIN;BSC_DIST_MAH_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_DIST_MAH, 80.0f);

/**
 * Configure "safety coefficient" used to multiply all battery usage calculations. Limited to [BSC_RTH_COEF_MIN;BSC_RTH_COEF_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_RTH_COEF, 1.10f);

/**
 * Battery level lpf frequency. Limited to [BSC_BAT_HZ_MIN;BSC_BAT_HZ_MAX].
 */
PARAM_DEFINE_FLOAT(A_BSC_BAT_HZ, 2.0f);
