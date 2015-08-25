#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * Enable battery safety check. 0 = disable, any non-zero value = enable.
 */
PARAM_DEFINE_INT32(A_SAH_DO, 1);

/**
 * Configure the allowed safety actions for the current activity.
 * This is a bit-mask, see Safety_action_helper::Safety_action for what each bit means.
 */
PARAM_DEFINE_INT32(A_SAH_ALLOWED, 258);

