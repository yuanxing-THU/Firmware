#pragma once

#include "navigator_mode.h"

enum CTM_STATE {
    CTM_STATE_NONE = 0,
    CTM_STATE_CLIMB_UP,
    CTM_STATE_RETURN,
    CTM_STATE_CLIMB_DOWN,
    CTM_STATE_DONE,
};

class CTM : public NavigatorMode
{
public:

	CTM(Navigator *navigator, const char *name);

	~CTM();

	virtual void on_inactive();

	virtual void on_activation();

	virtual void on_active();

	virtual void execute_vehicle_command();

private:

	void set_ctm_setpoint();
	void set_next_ctm_state();

    int _ctm_state;

    float _climb_alt;

    float _target_lat;
    float _target_lon;

};
