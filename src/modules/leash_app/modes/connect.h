#pragma once

#include "base.h"
#include "time.h"

namespace modes {

class ModeConnect : public Base
{
public:
    enum class State{
        UNKNOWN = 0,
        NOT_PAIRED,
        PAIRING,
        DISCONNECTED,
        CONNECTING,
        CHECK_MAVLINK,
        GETTING_ACTIVITIES,
        CONNECTED,
    };

    ModeConnect(State current = State::UNKNOWN);
    virtual ~ModeConnect();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);

private:
    const static int MAVLINK_CHECK_INTERVAL = 10; // in seconds

    bool forcing_pairing;

    State currentState;

    time_t startTime;

    // == methods ==
    void getConState();
    bool receiveActivityParams();
    void BTPairing(bool start = 1);
};

} //end of namespace modes
