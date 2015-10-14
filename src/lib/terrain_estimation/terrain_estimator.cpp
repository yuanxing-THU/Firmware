/****************************************************************************
 *
 *   Copyright (c) 2015 Roman Bapst. All rights reserved.
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

/**
 * @file terrain_estimator.cpp
 * A terrain estimation kalman filter.
 */

#include "terrain_estimator.h"

TerrainEstimator::TerrainEstimator() :
	_distance_last(0.0f),
	_terrain_valid(false),
	_time_last_distance(0),
	_time_last_gps(0)
{
	_x.zero();
	_u_z = 0.0f;
	_P.identity();
}

bool TerrainEstimator::is_distance_valid(float distance) {
	if (distance > 40.0f || distance < 0.00001f) {
		return false;
	} else {
		return true;
	}
}

void TerrainEstimator::predict(float dt, const struct vehicle_attitude_s *attitude, const struct sensor_combined_s *sensor,
		const struct distance_sensor_s *distance)
{
	if (attitude->R_valid) {
		math::Matrix<3, 3> R_att(attitude->R);
		math::Vector<3> a(&sensor->accelerometer_m_s2[0]);
		math::Vector<3> u;
		u = R_att * a;
		_u_z = u(2) + 9.81f; // compensate for gravity

	} else {
		_u_z = 0.0f;
	}

	// dynamics matrix
	math::Matrix<n_x, n_x> A;
	A.zero();
	A(0,1) = 1;
	A(1,2) = 1;

	// input matrix
	math::Matrix<n_x,1>  B;
	B.zero();
	B(1,0) = 1;

	// input noise variance
	float R = 0.135f;

	// process noise convariance
	math::Matrix<n_x, n_x>  Q;
	Q(0,0) = 0;
	Q(1,1) = 0;

	// do prediction
	math::Vector<n_x>  dx = (A * _x) * dt;
	dx(1) += B(1,0) * _u_z * dt;

	// propagate state and covariance matrix
	_x += dx;
	_P += (A * _P + _P * A.transposed() +
	       B * R * B.transposed() + Q) * dt;
}

void TerrainEstimator::measurement_update(const struct vehicle_gps_position_s *gps, const struct distance_sensor_s *distance,
				const struct vehicle_attitude_s *attitude)
{
	if (distance->timestamp > _time_last_distance) {

		float d = distance->current_distance;

		math::Matrix<1, n_x> C;
		C(0, 0) = -1; // measured altitude,

		float R = 0.009f;

		math::Vector<1> y;
		y(0) = d * cosf(attitude->roll) * cosf(attitude->pitch);

		// residual
		math::Matrix<1, 1> S_I = (C * _P * C.transposed());
		S_I(0,0) += R;
		S_I = S_I.inversed();
		math::Vector<1> r = y - C * _x;

		math::Matrix<n_x, 1> K = _P * C.transposed() * S_I;

		// some sort of outlayer rejection
		if (fabsf(distance->current_distance - _distance_last) < 1.0f) {
			_x += K * r;
			_P -= K * C * _P;
		}

		// if the current and the last range measurement are bad then we consider the terrain
		// estimate to be invalid
		if (!is_distance_valid(distance->current_distance) && !is_distance_valid(_distance_last)) {
			_terrain_valid = false;
		} else {
			_terrain_valid = true;
		}

		_time_last_distance = distance->timestamp;
		_distance_last = distance->current_distance;
	}

	if (gps->timestamp_position > _time_last_gps && gps->fix_type >= 3) {
		math::Matrix<1, n_x> C;
		C(0,1) = 1;

		float R = 0.056f;

		math::Vector<1> y;
		y(0) = gps->vel_d_m_s;

		// residual
		math::Matrix<1, 1> S_I = (C * _P * C.transposed());
		S_I(0,0) += R;
		S_I = S_I.inversed();
		math::Vector<1> r = y - C * _x;

		math::Matrix<n_x, 1> K = _P * C.transposed() * S_I;
		_x += K * r;
		_P -= K * C * _P;

		_time_last_gps = gps->timestamp_position;
	}

	// reinitialise filter if we find bad data
	bool reinit = false;
	for (int i = 0; i < n_x; i++) {
		if (!PX4_ISFINITE(_x(i))) {
			reinit = true;
		}
	}

	for (int i = 0; i < n_x; i++) {
		for (int j = 0; j < n_x; j++) {
			if (!PX4_ISFINITE(_P(i,j))) {
				reinit = true;
			}
		}
	}

	if (reinit) {
		_x.zero();
		_P.zero();
		_P(0,0) = _P(1,1) = _P(2,2) = 0.1f;
	}

}