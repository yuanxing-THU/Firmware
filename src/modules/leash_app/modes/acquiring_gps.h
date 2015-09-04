#pragma once

#include "base.h"

namespace modes
{

class Acquiring_gps : public Base
{
public:
    Acquiring_gps();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);
private:
    typedef enum
    {
        NO_GPS = 0,
        BAD_GPS,
        FAIR_GPS,
        GOOD_GPS,
        EXCELENT_GPS,
    } gps_qality;

    gps_qality airdogGPS;
    gps_qality leashGPS;
    void checkGPS();
    bool drone_has_gps;
    bool drone_has_home;
    bool leash_has_gps;
    bool leash_has_home;
    
    bool bothGotGPS();
};

}
