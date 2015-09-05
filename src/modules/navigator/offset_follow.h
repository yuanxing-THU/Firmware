/***************************************************************************
 *
 *   Copyright (c) 2014 PX4 Development Team. All rights reserved.
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
 * @file offset_follow.h
 */

#ifndef NAVIGATOR_OFFSET_FOLLOW_H
#define NAVIGATOR_OFFSET_FOLLOW_H

#include <controllib/blocks.hpp>
#include <controllib/block/BlockParam.hpp>

#include <uORB/uORB.h>

#include "navigator_mode.h"
#include "mission_block.h"

class OffsetFollow : public MissionBlock
{
public:

	OffsetFollow(Navigator *navigator, const char *name);

	~OffsetFollow();

	virtual void on_inactive();
	virtual void on_activation();
	virtual void on_active();
	virtual void execute_vehicle_command();

private:

    void on_active_circle_around();
    void on_active_abs_follow();
    void on_active_front_follow();

    void execute_vehicle_command_circle_around();
    void execute_vehicle_command_abs_follow();
    void execute_vehicle_command_front_follow();

    void init_base_offset();

    void update_offset_sp_angle();
    void update_follow_offset();

    void offset_height_step(int);
    void offset_distance_step(int);
    void offset_rotation_step(int, float&);

    void calc_actual_angle();
    void normalize_angle(float &angle);

    bool shortest_arc(float angle_from, float angle_to, float &angle, float &direction);

    math::Matrix<3, 3> R_phi;
    math::Vector<3> _follow_offset_vect;
    math::Vector<3> _base_offset;
    math::Vector<2> delta_pos;

	hrt_abstime _t_prev;
    hrt_abstime _t_cur;

    float dt;

    float _radius;

    float _offset_goal_angle;
    float _actual_angle;
    float _offset_sp_angle;

    float _rotation_speed;
    float _rotation_speed_ms;
    float _offset_len;

    float _angle_err;

    float _target_speed;

    const float _pi = (float)M_PI;
    const float _2pi = _pi * 2.0f;

    struct vehicle_status_s * _vstatus;

    bool _base_offset_inited;

    float _front_follow_aditional_angle;

};

#endif
