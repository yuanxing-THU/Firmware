
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <fcntl.h>

#include <mavlink/mavlink_log.h>
#include <systemlib/err.h>

#include <uORB/topics/position_setpoint_triplet.h>

#include "navigator.h"

#include "offset_follow.h"

OffsetFollow::OffsetFollow(Navigator *navigator, const char *name) :
	MissionBlock(navigator, name)
{
    updateParameters();
}

OffsetFollow::~OffsetFollow()
{
}

void
OffsetFollow::on_inactive()
{
}

void
OffsetFollow::on_activation()
{

	updateParameters();

	_navigator->invalidate_setpoint_triplet();

    follow_offset = _navigator->get_follow_offset();
    global_pos = _navigator->get_global_position();
	target_pos = _navigator->get_target_position();

    _vstatus = _navigator->get_vstatus();

    _rotation_speed_ms = 0.0f;

    printf("Offset follow mode start ! \n");

    init_follow_offset_vector(); 

    if (_vstatus->nav_state = NAVIGATION_STATE_CIRCLE_AROUND) {
        _rotation_speed_ms = _parameters.offset_rot_speed_ch_cmd_step;;
    } 
}

void
OffsetFollow::on_active()
{

    _vstatus = _navigator->get_vstatus();

    if ( update_vehicle_command() )
            execute_vehicle_command();

    switch(_vstatus->nav_state){

        case NAVIGATION_STATE_ABS_FOLLOW: 
            on_active_abs_follow();
            break;
        case NAVIGATION_STATE_FRONT_FOLLOW:
            on_active_front_follow();
            break;
        case NAVIGATION_STATE_CIRCLE_AROUND:
            on_active_circle_around();
            break;
        case NAVIGATION_STATE_KITE_LITE:
            // TODO
            break;
    }

    update_follow_offset();

}

void
OffsetFollow::on_active_abs_follow() {
    // Do nothing :)
}

void
OffsetFollow::on_active_front_follow() {

    _rotation_speed_ms = 0.0f;

    math::Vector<2> target_vel(target_pos->vel_n, target_pos->vel_e);

    float target_speed = target_vel.length();
    float min_angle_err = 0.0f;


    _rotation_angle_sp = - atan2(target_vel(0), target_vel(1));

    if (isnan(_rotation_angle_sp))
        _rotation_angle_sp = 0.0f;

    if (_rotation_angle_sp < 0.0f )
        _rotation_angle_sp+=_2pi;
    
    float rotation_dir = 0.0f;
    float angle_err_a = 0.0f;
    float angle_err_b = 0.0f;

    if (_rotation_angle_sp > _rotation_angle) {

        angle_err_a = _rotation_angle_sp - _rotation_angle;
        angle_err_b = _2pi - angle_err_a;

        if (angle_err_a < angle_err_b)  {
            rotation_dir = 1.0f;
            min_angle_err = angle_err_a;
        }
        else {
            rotation_dir = -1.0f;
            min_angle_err = angle_err_b;
        }
    } else {

        angle_err_a = _rotation_angle - _rotation_angle_sp;
        angle_err_b = _2pi - angle_err_a;

        if (angle_err_a < angle_err_b) {
            rotation_dir = -1.0f;
            min_angle_err = angle_err_a; 
        }
        else {
            rotation_dir = 1.0f;
            min_angle_err = angle_err_b;
        }
    }

    if (min_angle_err > _parameters.offset_angle_error_treshold) {
        _rotation_speed_ms = rotation_dir * target_speed * _parameters.offset_rot_speed_ratio;
    } else {
        _rotation_speed_ms = 0.0f;
    }

}

void
OffsetFollow::on_active_circle_around() {
}

void
OffsetFollow::execute_vehicle_command() {

	vehicle_command_s cmd = _vcommand;

	if (cmd.command == VEHICLE_CMD_DO_SET_MODE){

		//uint8_t base_mode = (uint8_t)cmd.param1;
		uint8_t main_mode = (uint8_t)cmd.param2;

		if (main_mode == PX4_CUSTOM_MAIN_MODE_RTL) {

			commander_request_s *commander_request = _navigator->get_commander_request();
			commander_request->request_type = V_MAIN_STATE_CHANGE;
			commander_request->main_state = MAIN_STATE_RTL;
			_navigator->set_commander_request_updated();

		}
	}
	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
		REMOTE_CMD remote_cmd = (REMOTE_CMD)cmd.param1;

		switch(remote_cmd){
			case REMOTE_CMD_PLAY_PAUSE: {
				commander_request_s *commander_request = _navigator->get_commander_request();
				commander_request->request_type = V_MAIN_STATE_CHANGE;
				commander_request->main_state = MAIN_STATE_LOITER;
				_navigator->set_commander_request_updated();
				break;
			}
			case  REMOTE_CMD_LAND_DISARM: {
                commander_request_s *commander_request = _navigator->get_commander_request();
                commander_request->request_type = V_MAIN_STATE_CHANGE;
                commander_request->main_state = MAIN_STATE_EMERGENCY_LAND;
                _navigator->set_commander_request_updated();
                break;
            }
		}
	}

    switch(_vstatus->nav_state){

        case NAVIGATION_STATE_ABS_FOLLOW: 
            execute_vehicle_command_abs_follow();
            break;
        case NAVIGATION_STATE_FRONT_FOLLOW:
            execute_vehicle_command_front_follow();
            break;
        case NAVIGATION_STATE_CIRCLE_AROUND:
            execute_vehicle_command_circle_around();
            break;
        case NAVIGATION_STATE_KITE_LITE:
            // TODO:
            break;
    }
}


void
OffsetFollow::execute_vehicle_command_abs_follow() {

    vehicle_command_s cmd = _vcommand;
    if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
        REMOTE_CMD remote_cmd = (REMOTE_CMD)cmd.param1;
        math::Vector<3> offset =_follow_offset_vect;

        switch(remote_cmd){
            case REMOTE_CMD_UP: 
                offset_height_step(-1);
                break;
            case REMOTE_CMD_DOWN: 
                offset_height_step(1);
                break;
            case REMOTE_CMD_LEFT: 
                offset_rotation_step(-1);
                break;
            case REMOTE_CMD_RIGHT: 
                offset_rotation_step(1);
                break;
            case REMOTE_CMD_CLOSER: 
                offset_distance_step(-1);
                break;
            case REMOTE_CMD_FURTHER:
                offset_distance_step(1);
                break;
        }
    }
}


void
OffsetFollow::execute_vehicle_command_circle_around() {
    vehicle_command_s cmd = _vcommand;

    if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
        REMOTE_CMD remote_cmd = (REMOTE_CMD)cmd.param1;

        switch(remote_cmd){
            case REMOTE_CMD_UP: 
                offset_height_step(-1);
                break;
            case REMOTE_CMD_DOWN: 
                offset_height_step(1);
                break;
            case REMOTE_CMD_LEFT: 

                _rotation_speed_ms += _parameters.offset_rot_speed_ch_cmd_step;
                break;
            case REMOTE_CMD_RIGHT: 
                _rotation_speed_ms -= _parameters.offset_rot_speed_ch_cmd_step;
                break;
            case REMOTE_CMD_CLOSER: 
                offset_distance_step(-1);
                break;
            case REMOTE_CMD_FURTHER:
                offset_distance_step(1);
                break;
        }
    }
}


void
OffsetFollow::execute_vehicle_command_front_follow() {

    vehicle_command_s cmd = _vcommand;

    if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
        REMOTE_CMD remote_cmd = (REMOTE_CMD)cmd.param1;

        switch(remote_cmd){
            case REMOTE_CMD_UP: 
                offset_height_step(-1);
                break;
            case REMOTE_CMD_DOWN: 
                offset_height_step(1);
                break;
            case REMOTE_CMD_CLOSER: 
                offset_distance_step(-1);
                break;
            case REMOTE_CMD_FURTHER:
                offset_distance_step(1);
                break;
        }
    }
}


void
OffsetFollow::offset_distance_step(int direction) {

    printf("Distance step\n");

    float step_len = _parameters.horizon_button_step;
    float cur_dst = sqrt(_follow_offset_vect(0) * _follow_offset_vect(0) + _follow_offset_vect(1) * _follow_offset_vect(1));
    float dst_left = 0.0f;

    if (direction == 1) {
        dst_left = _parameters.offset_max_distance - cur_dst;
    } else if (direction == -1) {
        dst_left = cur_dst - _parameters.offset_min_distance;
    }

    if (step_len > dst_left) step_len = dst_left;
    if (step_len < 0.0f) step_len = 0.0f;

    float alpha = atan2(_follow_offset_vect(1), _follow_offset_vect(0));

    if (isnan(alpha)) alpha = 1.0f;

    math::Vector<3> offset_delta(
        cosf(alpha) * step_len, 
        sinf(alpha) * step_len,
    0);

    if (direction == -1) offset_delta = -offset_delta;

    math::Vector<3> offset_new = _follow_offset_vect + offset_delta;
    _follow_offset_vect = offset_new;

    calculate_base_offset();
}

void
OffsetFollow::offset_rotation_step(int direction) {

    printf("Rotation step\n");

    math::Matrix<3, 3> R_phi;
    float alpha = _parameters.horizon_button_step / _radius;
    _rotation_angle += (float)direction * alpha;

}

void
OffsetFollow::offset_height_step(int direction) {

    printf("Height step\n");

    if (direction == -1)
        _follow_offset_vect(2) += direction * _parameters.up_button_step;

    if (direction == 1)
        _follow_offset_vect(2) += direction * _parameters.down_button_step;

    calculate_base_offset();
}

void
OffsetFollow::init_follow_offset_vector() {

    printf("Initing follow offset !\n");
    
    _t_prev = 0;
    _t_cur = 0;

    _rotation_angle = 0.0f;
    _rotation_speed = 0.0f;
    _offset_len = 1.0f;

	get_vector_to_next_waypoint(
			target_pos->lat,
			target_pos->lon,
			global_pos->lat,
			global_pos->lon,
            &_follow_offset_vect(0),
            &_follow_offset_vect(1)
	);

	_follow_offset_vect(2) = target_pos->alt - global_pos->alt;
    calculate_base_offset();
}

void
OffsetFollow::calculate_base_offset() {

    printf("Recalculating base offset !\n");

    _rotation_angle = -atan2(_follow_offset_vect(0), _follow_offset_vect(1));

    if (isnan(_rotation_angle))
        _rotation_angle = 0.0f;

    R_phi.from_euler(0.0f, 0.0f, -_rotation_angle);
    _base_offset = R_phi * _follow_offset_vect;

    _radius = sqrt(_base_offset(0) * _base_offset(0) + _base_offset(1) * _base_offset(1));

}

void
OffsetFollow::update_rotation_angle() {

    if (_rotation_speed_ms >= _parameters.max_offset_rot_speed)
        _rotation_speed_ms = _parameters.max_offset_rot_speed;

    if (_rotation_speed_ms <= -_parameters.max_offset_rot_speed)
        _rotation_speed_ms = -_parameters.max_offset_rot_speed;

    _rotation_speed = _rotation_speed_ms / _radius;

	_t_prev = _t_cur;
    _t_cur = hrt_absolute_time();
    dt = _t_prev != 0 ? (_t_cur - _t_prev) * 1e-6f : 0.0f;
    _rotation_angle += dt * _rotation_speed;

    if (_rotation_angle > _2pi) _rotation_angle -= _2pi;
    if (_rotation_angle < 0.0f) _rotation_angle += _2pi;

    // TODO: use _offset_len
    
}

void
OffsetFollow::update_follow_offset() {
    
    update_rotation_angle();

    R_phi.from_euler(0.0f, 0.0f, _rotation_angle);
    _follow_offset_vect = R_phi * _base_offset;

    follow_offset->x = _follow_offset_vect(0);
    follow_offset->y = _follow_offset_vect(1);
    follow_offset->z = _follow_offset_vect(2);

    _navigator -> publish_follow_offset();
}
