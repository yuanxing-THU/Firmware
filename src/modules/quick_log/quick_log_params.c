#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * @descr: Enable writing qlog to file. 1 = on, other = off.
 */
PARAM_DEFINE_INT32(A_QLOG_WRITE, 1);

/**
 * @descr: Enable dumping "qlog check" to debug port on boot. 1 = on, other = off.
 */
PARAM_DEFINE_INT32(A_QLOG_CHECK, 0);
