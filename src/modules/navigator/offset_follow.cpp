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
	MissionBlock(navigator, name),
    _follow_offset_vect(10.0f, 10.0f, 10.0f),
    _base_offset(10.0f, 10.0f, 10.0f),
    _t_prev(0),
    _t_cur(0),
    _radius(0.0f),
    _offset_goal_angle(0.0f),
    _actual_angle(0.0f),
    _offset_sp_angle(0.0f),
    _rotation_speed(0.0f),
    _rotation_speed_ms(0.0f),
    _offset_len(0.0f),
    _angle_err(0.0f),
    _vstatus(nullptr),
    _base_offset_inited(false)
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
    printf("Offset follow mode start ! \n");

	updateParameters();

	_navigator->invalidate_setpoint_triplet();

    follow_offset = _navigator->get_follow_offset();
    global_pos = _navigator->get_global_position();
	target_pos = _navigator->get_target_position();
    _vstatus = _navigator->get_vstatus();

    if (!_base_offset_inited)
        init_base_offset(); 

    calc_actual_angle();

    _offset_goal_angle = _offset_sp_angle = _actual_angle;

    _rotation_speed_ms = 0.0f;

    if (_vstatus->nav_state == NAVIGATION_STATE_CIRCLE_AROUND) {

        _rotation_speed_ms = NavigatorMode::parameters.offset_rot_speed_ch_cmd_step;

    } 
}

void
OffsetFollow::on_active()
{

    if ( update_vehicle_command() )
            execute_vehicle_command();

    calc_actual_angle();

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

    math::Vector<2> target_vel(target_pos->vel_n, target_pos->vel_e);

    _target_speed = target_vel.length();

    _offset_goal_angle = atan2( target_vel(1), target_vel(0));

    _offset_goal_angle += _front_follow_aditional_angle;

    normalize_angle(_offset_goal_angle);
    
    float direction = 0.0f;
    float min_angle_err = 0.0f;

    shortest_arc(_offset_sp_angle, _offset_goal_angle, min_angle_err, direction);

    if (min_angle_err > NavigatorMode::parameters.offset_angle_error_treshold) {
        _rotation_speed_ms = direction * _target_speed * NavigatorMode::parameters.offset_rot_speed_ratio;
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
                commander_request->main_state = MAIN_STATE_LAND;
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
                offset_rotation_step(-1, _offset_sp_angle);
                break;
            case REMOTE_CMD_RIGHT: 
                offset_rotation_step(1, _offset_sp_angle);
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

                _rotation_speed_ms -= NavigatorMode::parameters.offset_rot_speed_ch_cmd_step;
                break;
            case REMOTE_CMD_RIGHT: 
                _rotation_speed_ms += NavigatorMode::parameters.offset_rot_speed_ch_cmd_step;
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

            case REMOTE_CMD_LEFT: 
                offset_rotation_step(-1, _front_follow_aditional_angle);
                break;
            case REMOTE_CMD_RIGHT: 
                offset_rotation_step(1, _front_follow_aditional_angle);
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

    float step_len = NavigatorMode::parameters.horizon_button_step;
    float cur_dst = sqrt(_follow_offset_vect(0) * _follow_offset_vect(0) + _follow_offset_vect(1) * _follow_offset_vect(1));
    float dst_left = 0.0f;

    if (direction == 1) {
        dst_left = NavigatorMode::parameters.offset_max_distance - cur_dst;
    } else if (direction == -1) {
        dst_left = cur_dst - NavigatorMode::parameters.offset_min_distance;
    }

    if (step_len > dst_left) step_len = dst_left;
    if (step_len < 0.0f) step_len = 0.0f;

    _base_offset(0) += direction * step_len;

}

void
OffsetFollow::offset_rotation_step(int direction, float &angle) {

    printf("Rotation step\n");

    float alpha = NavigatorMode::parameters.horizon_button_step / _radius;
    angle += direction * alpha;
    normalize_angle(angle);

}

void
OffsetFollow::offset_height_step(int direction) {

    printf("Height step\n");

    if (direction == -1)
        _base_offset(2) += direction * NavigatorMode::parameters.up_button_step;

    if (direction == 1)
        _base_offset(2) += direction * NavigatorMode::parameters.down_button_step;

}

void
OffsetFollow::init_base_offset() {

    printf("Initing base offset !\n");

	get_vector_to_next_waypoint(
			target_pos->lat,
			target_pos->lon,
			global_pos->lat,
			global_pos->lon,
            &_base_offset(0),
            &_base_offset(1)
	);

	_base_offset(2) = target_pos->alt - global_pos->alt;

    _offset_sp_angle = atan2(_base_offset(1), _base_offset(0));

    // Rotate base offset to zero
    R_phi.from_euler(0.0f, 0.0f, -_offset_sp_angle);
    _base_offset = R_phi * _base_offset;

    _base_offset(0) = NavigatorMode::parameters.offset_initial_distance;

    _radius = sqrt(_base_offset(0) * _base_offset(0) + _base_offset(1) * _base_offset(1));

    _offset_len = 1.0f;

    _base_offset_inited = true;

    _front_follow_aditional_angle = NavigatorMode::parameters.front_follow_additional_angle;

}

void
OffsetFollow::update_offset_sp_angle() {

    if (_rotation_speed_ms >= NavigatorMode::parameters.max_offset_rot_speed)
        _rotation_speed_ms = NavigatorMode::parameters.max_offset_rot_speed;
        
    if (_rotation_speed_ms <= -NavigatorMode::parameters.max_offset_rot_speed)
        _rotation_speed_ms = -NavigatorMode::parameters.max_offset_rot_speed;

    _rotation_speed = _rotation_speed_ms / _radius;

    _t_prev = _t_cur;
    _t_cur = hrt_absolute_time();
    dt = _t_prev != 0 ? (_t_cur - _t_prev) * 1e-6f : 0.0f;
    _offset_sp_angle += dt * _rotation_speed;


    float offset_sp_angle_err = 0.0f;
    float direction = 0.0f;

    shortest_arc(_actual_angle, _offset_sp_angle, offset_sp_angle_err, direction);

    if (offset_sp_angle_err > NavigatorMode::parameters.max_offset_sp_angle_err) {
    
        _offset_sp_angle = _actual_angle + direction * NavigatorMode::parameters.max_offset_sp_angle_err;
    
    }

    normalize_angle(_offset_sp_angle);

}

void
OffsetFollow::calc_actual_angle(){
    
	get_vector_to_next_waypoint(
			target_pos->lat,
			target_pos->lon,
			global_pos->lat,
			global_pos->lon,
            &delta_pos(0),
            &delta_pos(1)
	);

    _actual_angle = atan2(delta_pos(1), delta_pos(0));

    normalize_angle(_actual_angle);
}


void
OffsetFollow::update_follow_offset() {
    
    update_offset_sp_angle();

    R_phi.from_euler(0.0f, 0.0f, _offset_sp_angle);
    _follow_offset_vect = R_phi * _base_offset;

    follow_offset->x = _follow_offset_vect(0);
    follow_offset->y = _follow_offset_vect(1);
    follow_offset->z = _follow_offset_vect(2);

    _navigator -> publish_follow_offset();
}


bool
OffsetFollow::shortest_arc(float angle_from, float angle_to, float &angle, float &direction){

    float angle_err_a = 0.0f;
    float angle_err_b = 0.0f;

    if (angle_to > angle_from) {

        angle_err_a = angle_to - angle_from;
        angle_err_b = _2pi - angle_err_a;

        if (angle_err_a < angle_err_b)  {
            direction = 1.0f;
            angle = angle_err_a;
        }
        else {
            direction = -1.0f;
            angle = angle_err_b;
        }

    } else {

        angle_err_a = angle_from - angle_to;
        angle_err_b = _2pi - angle_err_a;

        if (angle_err_a < angle_err_b) {
            direction = -1.0f;
            angle = angle_err_a; 
        }
        else {
            direction = 1.0f;
            angle = angle_err_b;
        }
    }

    return true;
}


void
OffsetFollow::normalize_angle(float &angle) {

    while (angle < 0.0f)
        angle += _2pi;

    while (angle > _2pi)
        angle -= _2pi;

} 
