#pragma once

#include "base.h"

namespace modes
{

class SensorCheck : public Base
{
public:
    SensorCheck();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);
private:

};

}
