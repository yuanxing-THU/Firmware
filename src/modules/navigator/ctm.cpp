/**
 * Come to me mode
 * @author Martins Frolovs <martins.f@airdog.com>
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>

#include <mavlink/mavlink_log.h>
#include <systemlib/err.h>
#include <geo/geo.h>

#include "navigator.h"
#include "ctm.h"


CTM::CTM(Navigator *navigator, const char *name) :
    NavigatorMode(navigator, name),
    _ctm_state(CTM_STATE_NONE)
{
}

CTM::~CTM()
{
}

void
CTM::on_inactive()
{
}

void
CTM::on_activation()
{

	_navigator->invalidate_setpoint_triplet();
	global_pos = _navigator->get_global_position();
    target_pos = _navigator->get_target_position();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

    float drone_start_alt = _navigator -> get_drone_start_alt();
    float target_start_alt = _navigator -> get_target_start_alt();

    float talt_offs = NavigatorMode::parameters.follow_talt_offs;

    float target_alt_delta = target_pos->alt - target_start_alt;

    // (drone_start_alt + talt_offs) is target altitude on start from drone point of view 
    _climb_alt = drone_start_alt + talt_offs + target_alt_delta + NavigatorMode::parameters.takeoff_alt;

    _target_lat = target_pos->lat;
    _target_lon = target_pos->lon;

	float xy_distance = get_distance_to_next_waypoint(
			global_pos->lat, 
            global_pos->lon,
            _target_lat,
            _target_lon
	);

	/* Vehicle is already above the head */
	if (xy_distance <= NavigatorMode::parameters.acceptance_radius && global_pos->alt - 1.0f <= _climb_alt && global_pos->alt + 1.0f >= _climb_alt) {
		_ctm_state = CTM_STATE_DONE;
	/* Above the head need to descend */
	} else if (xy_distance <= NavigatorMode::parameters.acceptance_radius) {
		_ctm_state = CTM_STATE_CLIMB_DOWN;
	/* No need to climb up - return to target */
	} else if ( global_pos->alt >= _climb_alt ) {
		_ctm_state = CTM_STATE_RETURN;
	} else {
		_ctm_state = CTM_STATE_CLIMB_UP;
	}

	set_camera_mode(HORIZONTAL, true);
	set_ctm_setpoint();

}

void
CTM::on_active()
{
	if ( update_vehicle_command() )
			execute_vehicle_command();

	if (_ctm_state != CTM_STATE_DONE ) {

		if (check_current_pos_sp_reached(SETPOINT_TYPE_POSITION)) {
			set_next_ctm_state();
			set_ctm_setpoint();
		}
	}
}

void
CTM::execute_vehicle_command()
{
	vehicle_command_s cmd = _vcommand;

	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {

		int remote_cmd = cmd.param1;

		if (remote_cmd == REMOTE_CMD_PLAY_PAUSE) {

			commander_request_s *commander_request = _navigator->get_commander_request();
			commander_request->request_type = V_MAIN_STATE_CHANGE;
			commander_request->main_state = MAIN_STATE_LOITER;
			_navigator->set_commander_request_updated();

		}
	}
}

void
CTM::set_ctm_setpoint()
{

    pos_sp_triplet->previous.valid = false;
    pos_sp_triplet->current.valid = true;
    pos_sp_triplet->next.valid = false;

	switch (_ctm_state) {

		case CTM_STATE_CLIMB_UP: {


			pos_sp_triplet->current.lat = global_pos->lat;
			pos_sp_triplet->current.lon = global_pos->lon;
			pos_sp_triplet->current.alt = _climb_alt > global_pos->alt ? _climb_alt : global_pos->alt;
			pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
			pos_sp_triplet->current.position_valid = true;

			break;
		}
		case CTM_STATE_RETURN: {


			pos_sp_triplet->current.lat = _target_lat;
			pos_sp_triplet->current.lon = _target_lon;
		    pos_sp_triplet->current.alt = global_pos->alt;
			pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
			pos_sp_triplet->current.position_valid = true;;

			break;
		}
		case CTM_STATE_CLIMB_DOWN: {


			pos_sp_triplet->current.lat = global_pos->lat;
			pos_sp_triplet->current.lon = global_pos->lon;
			pos_sp_triplet->current.alt = _climb_alt;
			pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
			pos_sp_triplet->current.position_valid = true;

			break;
		}
		case CTM_STATE_DONE: {


			commander_request_s *commander_request = _navigator->get_commander_request();
			commander_request->request_type = V_MAIN_STATE_CHANGE;
			commander_request->main_state = MAIN_STATE_LOITER;
			_navigator->set_commander_request_updated();

			break;
		}
		default:
			break;
	}

	_navigator->set_position_setpoint_triplet_updated();

}

void
CTM::set_next_ctm_state()
{
	switch (_ctm_state) {
		case CTM_STATE_CLIMB_UP:
			_ctm_state = CTM_STATE_RETURN;
			break;

		case CTM_STATE_RETURN:
			_ctm_state = CTM_STATE_CLIMB_DOWN;
			break;

		case CTM_STATE_CLIMB_DOWN:
			_ctm_state = CTM_STATE_DONE;
			break;
		default:
			break;
	}
}
