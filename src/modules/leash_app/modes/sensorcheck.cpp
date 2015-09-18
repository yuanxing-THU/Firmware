#include "sensorcheck.h"
#include "../datamanager.h"

namespace modes
{

int SensorCheck::getTimeout()
{
    return -1;
}

void SensorCheck::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_AirdogStatus] = 1;
    awaitMask[FD_VehicleStatus] = 1;
}

Base* SensorCheck::doEvent(int orbId)
{
    if (orbId == FD_AirdogStatus)
    {

    }
    else if (orbId == FD_VehicleStatus)
    {

    }
}

}
