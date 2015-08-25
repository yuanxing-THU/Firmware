#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * Enable battery safety check. 0 = disable, any non-zero value = enable.
 */
PARAM_DEFINE_INT32(A_SAH_DO, 1);

/**
 * Configure the allowed safety actions for the current activity. This should be the action codes ORed together.
 * 1 = land on spot only, 2 = return to home only, 3 = land on spot or return to home.
 */
PARAM_DEFINE_INT32(A_SAH_ALLOWED, 2);

