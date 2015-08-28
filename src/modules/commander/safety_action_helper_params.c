#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * Enable battery safety check. 0 = disable, any non-zero value = enable.
 */
PARAM_DEFINE_INT32(A_SAH_DO, 1);

/**
 * Configure whether spot landing is disallowed. 1 = yes, 0 = no.
 */
PARAM_DEFINE_INT32(A_SAH_NO_SPOT, 0);

/**
 * Configure whether rth is allowed in the current activity. 1 = yes, 0 = no.
 */
PARAM_DEFINE_INT32(A_SAH_RTH, 1);

/**
 * Configure whether leash control is allowed after an emergency in the current activity. 1 = yes, 0 = no.
 */
PARAM_DEFINE_INT32(A_SAH_CTRL, 1);

