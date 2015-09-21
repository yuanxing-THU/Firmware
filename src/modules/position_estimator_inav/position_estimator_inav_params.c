/****************************************************************************
 *
 *   Copyright (c) 2013, 2014 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/*
 * @file position_estimator_inav_params.c
 *
 * @author Anton Babushkin <rk3dov@gmail.com>
 *
 * Parameters for position_estimator_inav
 */

#include "position_estimator_inav_params.h"

/* Delta of range finder noise.
 * Changes smaller that this will be considered as noise and LPFed with coeff SENS_LID_L_LPF
 * Changes higher that this will be considered as height change and LPFed with coeff SENS_LID_L_LPF
 */
PARAM_DEFINE_FLOAT(SENS_LID_CUT, 0.7f);

/* LPF coefficient to use for great incoming changes
 * (GREATER than SENS_LID_CUT)
 */
PARAM_DEFINE_FLOAT(SENS_LID_H_LPF, 1.5f);

/* LPF coefficient to use for small incoming changes
 * (SMALLER than SENS_LID_CUT)
 */
PARAM_DEFINE_FLOAT(SENS_LID_L_LPF, 0.05f);

/**
 * Accepted for arming drift speed over all axes
 *
 * @min 0.0
 * @max unlimited
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_OK_DRIFT, 0.2f);

/**
 * Z axis weight for barometer
 *
 * Weight (cutoff frequency) for barometer altitude measurements.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_Z_BARO, 0.5f);

/**
 * Z axis weight for GPS
 *
 * Weight (cutoff frequency) for GPS altitude measurements. GPS altitude data is very noisy and should be used only as slow correction for baro offset.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_Z_GPS_P, 0.001f);

/**
 * Z axis weight for vision
 *
 * Weight (cutoff frequency) for vision altitude measurements. vision altitude data is very noisy and should be used only as slow correction for baro offset.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_Z_VIS_P, 0.5f);

/**
 * Z axis weight for sonar
 *
 * Weight (cutoff frequency) for sonar measurements.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_Z_SONAR, 3.0f);

/**
 * XY axis weight for GPS position
 *
 * Weight (cutoff frequency) for GPS position measurements.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_XY_GPS_P, 
#ifdef CONFIG_ARCH_BOARD_AIRLEASH
		1.0f
#else
		0.5f
#endif
);


/**
 * XY axis weight for GPS velocity
 *
 * Weight (cutoff frequency) for GPS velocity measurements.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_XY_GPS_V, 
#ifdef CONFIG_ARCH_BOARD_AIRLEASH
		2.0f
#else
		1.0f
#endif
);

/**
 * XY axis weight for vision position
 *
 * Weight (cutoff frequency) for vision position measurements.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_XY_VIS_P, 5.0f);

/**
 * XY axis weight for vision velocity
 *
 * Weight (cutoff frequency) for vision velocity measurements.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_XY_VIS_V, 0.0f);

/**
 * XY axis weight for optical flow
 *
 * Weight (cutoff frequency) for optical flow (velocity) measurements.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_XY_FLOW, 5.0f);

/**
 * XY axis weight for resetting velocity
 *
 * When velocity sources lost slowly decrease estimated horizontal velocity with this weight.
 *
 * @min 0.0
 * @max 10.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_XY_RES_V, 0.5f);

/**
 * XY axis weight factor for GPS when optical flow available
 *
 * When optical flow data available, multiply GPS weights (for position and velocity) by this factor.
 *
 * @min 0.0
 * @max 1.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_GPS_FLOW, 0.1f);

/**
 * Accelerometer bias estimation weight
 *
 * Weight (cutoff frequency) for accelerometer bias estimation. 0 to disable.
 *
 * @min 0.0
 * @max 0.1
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_W_ACC_BIAS, 0.05f);

/**
 * Optical flow scale factor
 *
 * Factor to convert raw optical flow (in pixels) to radians [rad/px].
 *
 * @min 0.0
 * @max 1.0
 * @unit rad/px
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_FLOW_K, 0.15f);

/**
 * Minimal acceptable optical flow quality
 *
 * 0 - lowest quality, 1 - best quality.
 *
 * @min 0.0
 * @max 1.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_FLOW_Q_MIN, 0.5f);

/**
 * Land detector time
 *
 * Vehicle assumed landed if no altitude changes happened during this time on low throttle.
 *
 * @min 0.0
 * @max 10.0
 * @unit s
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_LAND_T, 3.0f);

/**
 * Land detector altitude dispersion threshold
 *
 * Dispersion threshold for triggering land detector.
 *
 * @min 0.0
 * @max 10.0
 * @unit m
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_LAND_DISP, 0.7f);

/**
 * Land detector throttle threshold
 *
 * Value should be lower than minimal hovering thrust. Half of it is good choice.
 *
 * @min 0.0
 * @max 1.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_LAND_THR, 0.35f);

/**
 * GPS delay
 *
 * GPS delay compensation
 *
 * @min 0.0
 * @max 1.0
 * @unit s
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_DELAY_GPS, 0.056f);

/**
 * Disable vision input
 *
 * Set to the appropriate key (328754) to disable vision input.
 *
 * @min 0.0
 * @max 1.0
 * @group Position Estimator INAV
 */
PARAM_DEFINE_INT32(CBRK_NO_VISION, 0);

/**
 * EPH needed to init reference position
 *
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_INIT_EPH,
#ifdef CONFIG_ARCH_BOARD_AIRLEASH
		3.5f
#else
		2.0f
#endif
);


/**
 * EPV needed to init reference position
 *
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_INIT_EPV,
#ifdef CONFIG_ARCH_BOARD_AIRLEASH
		4.5f
#else
		2.5f
#endif
);


/**
 * Minimal time ms needed to init reference position
 *
 * @group Position Estimator INAV
 */
PARAM_DEFINE_INT32(INAV_INIT_WAIT, 15000);

/**
 * Flight time EPH when GPS considered valid
 * Above this value considered inavlid
 * Bellow this value *0.7 is considered valid
 *
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_GPS_OK_EPH, 12.0f);

/**
 * Flight time EPV when GPS considered valid
 * Above this value considered inavlid
 * Bellow this value *0.7 is considered valid
 *
 * @group Position Estimator INAV
 */
PARAM_DEFINE_FLOAT(INAV_GPS_OK_EPV, 15.0f);

/**
 * LPF cutoff frequency for X velocity filtering
 */
PARAM_DEFINE_FLOAT(INAV_VEL_X_LPF, 0.0f);

/**
 * LPF cutoff frequency for Y velocity filtering
 */
PARAM_DEFINE_FLOAT(INAV_VEL_Y_LPF, 0.0f);

/**
 * LPF cutoff frequency for Z velocity filtering
 */
PARAM_DEFINE_FLOAT(INAV_VEL_Z_LPF,
#ifdef CONFIG_ARCH_BOARD_AIRLEASH
		10.0f
#else
		0.0f
#endif
);


int parameters_init(struct position_estimator_inav_param_handles *h)
{
    h->lid_cut = param_find("SENS_LID_CUT");
    h->lid_h_lpf = param_find("SENS_LID_H_LPF");
    h->lid_l_lpf = param_find("SENS_LID_L_LPF");
    h->ok_drift = param_find("INAV_OK_DRIFT");
	h->w_z_baro = param_find("INAV_W_Z_BARO");
	h->w_z_gps_p = param_find("INAV_W_Z_GPS_P");
	h->w_z_vision_p = param_find("INAV_W_Z_VIS_P");
	h->w_z_sonar = param_find("INAV_W_Z_SONAR");
	h->w_xy_gps_p = param_find("INAV_W_XY_GPS_P");
	h->w_xy_gps_v = param_find("INAV_W_XY_GPS_V");
	h->w_xy_vision_p = param_find("INAV_W_XY_VIS_P");
	h->w_xy_vision_v = param_find("INAV_W_XY_VIS_V");
	h->w_xy_flow = param_find("INAV_W_XY_FLOW");
	h->w_xy_res_v = param_find("INAV_W_XY_RES_V");
	h->w_gps_flow = param_find("INAV_W_GPS_FLOW");
	h->w_acc_bias = param_find("INAV_W_ACC_BIAS");
	h->flow_k = param_find("INAV_FLOW_K");
	h->flow_q_min = param_find("INAV_FLOW_Q_MIN");
	h->sonar_err = param_find("SENS_SON_ERR");
    h->sonar_on = param_find("SENS_SON_ON");
	h->land_t = param_find("INAV_LAND_T");
	h->land_disp = param_find("INAV_LAND_DISP");
    h->land_min_h = param_find("A_LAND_SAFE_H");
	h->land_thr = param_find("INAV_LAND_THR");
	h->no_vision = param_find("CBRK_NO_VISION");
	h->delay_gps = param_find("INAV_DELAY_GPS");
	h->gps_init_eph = param_find("INAV_INIT_EPH");
	h->gps_init_epv = param_find("INAV_INIT_EPV");
	h->gps_init_wait = param_find("INAV_INIT_WAIT");
	h->gps_ok_eph = param_find("INAV_GPS_OK_EPH");
	h->gps_ok_epv = param_find("INAV_GPS_OK_EPV");
	h->vel_x_cutoff = param_find("INAV_VEL_X_LPF");
	h->vel_y_cutoff = param_find("INAV_VEL_Y_LPF");
	h->vel_z_cutoff = param_find("INAV_VEL_Z_LPF");

	return OK;
}

int parameters_update(const struct position_estimator_inav_param_handles *h, struct position_estimator_inav_params *p)
{
    param_get(h->lid_cut, &(p->lid_cut));
    param_get(h->lid_h_lpf, &(p->lid_h_lpf));
    param_get(h->lid_l_lpf, &(p->lid_l_lpf));
    param_get(h->ok_drift, &(p->ok_drift));
	param_get(h->w_z_baro, &(p->w_z_baro));
	param_get(h->w_z_gps_p, &(p->w_z_gps_p));
	param_get(h->w_z_vision_p, &(p->w_z_vision_p));
	param_get(h->w_z_sonar, &(p->w_z_sonar));
	param_get(h->w_xy_gps_p, &(p->w_xy_gps_p));
	param_get(h->w_xy_gps_v, &(p->w_xy_gps_v));
	param_get(h->w_xy_vision_p, &(p->w_xy_vision_p));
	param_get(h->w_xy_vision_v, &(p->w_xy_vision_v));
	param_get(h->w_xy_flow, &(p->w_xy_flow));
	param_get(h->w_xy_res_v, &(p->w_xy_res_v));
	param_get(h->w_gps_flow, &(p->w_gps_flow));
	param_get(h->w_acc_bias, &(p->w_acc_bias));
	param_get(h->flow_k, &(p->flow_k));
	param_get(h->flow_q_min, &(p->flow_q_min));
	param_get(h->sonar_err, &(p->sonar_err));
    param_get(h->sonar_on, &(p->sonar_on));
	param_get(h->land_t, &(p->land_t));
	param_get(h->land_disp, &(p->land_disp));
    param_get(h->land_min_h, &(p->land_min_h));
	param_get(h->land_thr, &(p->land_thr));
	param_get(h->no_vision, &(p->no_vision));
	param_get(h->delay_gps, &(p->delay_gps));
	param_get(h->gps_init_eph, &(p->gps_init_eph));
	param_get(h->gps_init_epv, &(p->gps_init_epv));
	param_get(h->gps_init_wait, &(p->gps_init_wait));
	param_get(h->gps_ok_eph, &(p->gps_ok_eph));
	param_get(h->gps_ok_epv, &(p->gps_ok_epv));
	param_get(h->vel_x_cutoff, &(p->vel_x_cutoff));
	param_get(h->vel_y_cutoff, &(p->vel_y_cutoff));
	param_get(h->vel_z_cutoff, &(p->vel_z_cutoff));

	return OK;
}
