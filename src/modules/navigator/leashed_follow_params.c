#include <nuttx/config.h>

#include <systemlib/param/param.h>

/**
 * cable park mode acceleration
 *
 * @unit meters/sec
 * @group Airdog params, cablepark
 */
PARAM_DEFINE_FLOAT(A_CP_MIN_ACC, 1.0f);
