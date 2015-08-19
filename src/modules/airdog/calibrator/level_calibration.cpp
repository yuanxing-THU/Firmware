#include <nuttx/config.h>

#include <drivers/drv_hrt.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <systemlib/param/param.h>
#include <uorb/topics/vehicle_attitude.h>
#include <uorb/uorb.h>

#include "calibration_commons.hpp"

namespace calibration {
// TODO! [AK] Rework hardcoded knowledge of board rotation that swaps X and Y and negates roll.
const char* pitch_param = "SENS_BOARD_X_OFF";
const char* roll_param = "SENS_BOARD_Y_OFF";

inline bool cleanup(int fd, bool reset_calibration=false, float saved_pitch=0.0f, float saved_roll=0.0f) {
	bool res = true;
	if (fd >= 0) {
		close(fd);
	}
	if (reset_calibration) {
		res = (param_set(param_find(pitch_param), &saved_pitch) == 0
			&& param_set(param_find(roll_param), &saved_roll) == 0);
	}
	return res;
};

// TODO! [AK] Parameters!

CALIBRATION_RESULT do_calibrate_level() {
	int att_sub = orb_subscribe(ORB_ID(vehicle_attitude));
	if (att_sub < 0) {
		return CALIBRATION_RESULT::FAIL;
	}
	vehicle_attitude_s attitude;
	double mean_pitch = 0.0, mean_roll = 0.0;
	int sample_cnt = 0;
	float saved_pitch=0.0f, saved_roll=0.0f;
	bool ok;
	ok = (param_get(param_find(pitch_param), &saved_pitch) == 0
		&& param_get(param_find(roll_param), &saved_roll) == 0);
	if (ok) {
		// Relies on the fact that means are 0 at start. Hacky.
		ok = (param_set(param_find(pitch_param), &mean_pitch) == 0
			&& param_set(param_find(roll_param), &mean_roll) == 0);
		if (!ok) {
			cleanup(att_sub, true, saved_pitch, saved_roll);
			return CALIBRATION_RESULT::SCALE_RESET_FAIL;
		}
	}
	else {
		cleanup(att_sub);
		return CALIBRATION_RESULT::SCALE_READ_FAIL;
	}

	// TODO! [AK] Replace hardcoded pause with attitude change rate check
	// Pause to allow attitude to settle after rotation reset
	sleep(30);

	pollfd poll_data[1];
	poll_data[0].fd = att_sub;
	poll_data[0].events = POLLIN;

	hrt_abstime end_time = hrt_absolute_time() + 5000000;

	int res = 0;
	int err_cnt = 0;

	while (hrt_absolute_time() < end_time && err_cnt < 50) {
		res = poll(poll_data, sizeof(poll_data) / sizeof(poll_data[0]), 50000);
		if (res >= 1) {
			orb_copy(ORB_ID(vehicle_attitude), att_sub, &attitude);
			if (isfinite(attitude.pitch + attitude.roll)) {
				mean_pitch += (double) attitude.pitch;
				mean_roll += (double) attitude.roll;
				++sample_cnt;
			}
			else {
				++err_cnt;
			}
		}
		else {
			++err_cnt;
		}
	}
	if (err_cnt < 50 && sample_cnt >= 30) {
		mean_pitch /= sample_cnt;
		mean_roll /= sample_cnt;
		printf("res p: %9.6f, res r: %9.6f\n", mean_pitch, mean_roll);
		mean_pitch *= (double)M_RAD_TO_DEG;
		mean_roll *= (double)M_RAD_TO_DEG;
		printf("res p: %9.6f, res r: %9.6f\n", mean_pitch, mean_roll);
		if (mean_pitch*mean_pitch + mean_roll*mean_roll > 7.0*7.0) {
			cleanup(att_sub, true, saved_pitch, saved_roll);
			return CALIBRATION_RESULT::SENSOR_DATA_FAIL;
		}
		// TODO! [AK] Rework hardcoded knowledge of the board rotation that requires roll sign swap
		if (cleanup(att_sub, true, mean_pitch, -mean_roll)) {
			if (param_save_default() != 0) {
				return CALIBRATION_RESULT::PARAMETER_DEFAULT_FAIL;
			}
			return CALIBRATION_RESULT::SUCCESS;
		}
		else {
			cleanup(att_sub, true, saved_pitch, saved_roll);
			return CALIBRATION_RESULT::PARAMETER_SET_FAIL;
		}
	}
	else {
		cleanup(att_sub, true, saved_pitch, saved_roll);
		return CALIBRATION_RESULT::SENSOR_DATA_FAIL;
	}
}

} // end namespace calibration
