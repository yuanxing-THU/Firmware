#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * GPS message delay interval in ms
 * If set to 0, will use default value in ubx.h
 *
 * @min 0
 * @group GPS
 */
PARAM_DEFINE_INT32(GPS_UBX_INTERVAL, 56);

/**
 * GPS dynamics model
 * 0 Portable, 2 Stationary, 3 Pedestrian, 4 Automotive, 5 Sea,
 * 6 Airborne <1g, 7 Airborne <2g, 8 Airborne <4g
 * If set to -1, will use default value in ubx.h
 *
 * @min -1
 * @max 8
 * @group GPS
 */
PARAM_DEFINE_INT32(GPS_UBX_DYNAMICS, -1);


/**
 * GPS min tracking channels
 * -1 - use defaul
 *
 * @min 0
 * @max 32
 * @group GPS
 */
PARAM_DEFINE_INT32(UBX_GPS_MIN_CHN, 8);

/**
 * GPS max tracking channels
 * -1 - use defaul
 *
 * @min 0
 * @max 32
 * @group GPS
 */
PARAM_DEFINE_INT32(UBX_GPS_MAX_CHN, 24);

/**
 * GPS enable GPS and QZSS
 * 0 - disable GPS and QZSS
 * 1 - enable GPS and
 *
 * @min 0
 * @max 1
 * @group GPS
 */
PARAM_DEFINE_INT32(UBX_ENABLE_GPS, 1);

/**
 * GPS enable SBAS
 * 0 - disable SBAS
 * 1 - enable SBAS
 *
 * @min 0
 * @max 1
 * @group GPS
 */
PARAM_DEFINE_INT32(UBX_ENABLE_SBAS, 1);

/**
 * GPS enable GLONASS
 * 0 - disable GLONASS
 * 1 - enable GLONASS
 *
 * @min 0
 * @max 1
 * @group GPS
 */
PARAM_DEFINE_INT32(UBX_ENABLE_GLONASS, 0);
