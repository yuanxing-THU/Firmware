
#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * Minimum offset length on XY plane in Offset follow modes
 * @unit m
 * @min 0.0
 */
PARAM_DEFINE_FLOAT(OFF_DST_MIN, 7.0f);

/**
 * Maximum offset length on XY plane in Offset follow modes
 * @unit m
 * @min 50.0
 */
PARAM_DEFINE_FLOAT(OFF_DST_MAX, 50.0f);

/**
 * Maximum offset rotation speed in Offset follow modes
 * @unit m/s
 */
PARAM_DEFINE_FLOAT(OFF_MAX_ROT_SPD, 8.0f);

/**
 * Offset angle error treshold. 
 * In FRONT FOLLOW angle error will have to 
 * exceed OFF_ANGLE_ERR_T to change following angle.
 * @unit rad
 */
PARAM_DEFINE_FLOAT(OFF_ANGL_ERR_T, 0.3f);

/**
 * Offset rotation speed ratio. 
 * In FRONT FOLLOW offset rotation speed will be calculated as follows:
 * target_speed * OFF_ROT_SPD_R
 */
PARAM_DEFINE_FLOAT(OFF_ROT_SPD_R, 3.0f);

/**
 * Offset rotation speed change command step in CIRCLE AROUND mode.
 * @unit m/s
 */
PARAM_DEFINE_FLOAT(OFF_ROT_SPD_STP, 2.0f);

/**
 * Aditional angle in front follow
 */
PARAM_DEFINE_FLOAT(OFF_FR_ADD_ANG, 0.0f);

/**
 * Maximum delta between actual target to drone angle and sp angle
 * @unit rad
 */
PARAM_DEFINE_FLOAT(OFF_MAX_SP_ANG_D, 2.0f);

/**
 * Initial distance for offset follow modes
 * @unit m
 */
PARAM_DEFINE_FLOAT(OFF_INTL_DST, 8.0f);
