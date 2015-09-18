#include "connect.h"
#include "menu.h"
#include "acquiring_gps.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "../datamanager.h"
#include "../button_handler.h"
#include "../displayhelper.h"

#include "../../mavlink/mavlink_defines.h"

#include <commander/commander_error.h>

#define _BLUETOOTH21_BASE       0x2d00

#define PAIRING_ON          _IOC(_BLUETOOTH21_BASE, 0)
#define PAIRING_OFF         _IOC(_BLUETOOTH21_BASE, 1)


namespace modes
{

ModeConnect::ModeConnect(State Current) : 
    forcing_pairing(false),
    startTime(0)
{
    if (Current == State::UNKNOWN)
    {
        getConState();
    }
    else
    {
        setState(Current);

        if (Current != State::PAIRING)
        {
            doEvent(-1);
        }
    }
}

ModeConnect::~ModeConnect() { }

void ModeConnect::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_BLRHandler] = 1;

    switch (currentState)
    {
        case State::GETTING_ACTIVITIES:
            awaitMask[FD_ActivityParams] = 1;
            break;

        case State::CHECK_MAVLINK:
            awaitMask[FD_MavlinkStatus] = 1;
            break;
    }

    awaitMask[FD_KbdHandler] = 1;
}

int ModeConnect::getTimeout()
{
    return -1;
}

Base* ModeConnect::doEvent(int orbId)
{
    Base *nextMode = nullptr;
    if (forcing_pairing)
    {
        if (currentState == State::PAIRING)
            forcing_pairing = false;
    }
    else if (currentState != State::GETTING_ACTIVITIES && orbId == FD_BLRHandler)
    {
        getConState();
    }

    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_MODE))
        {
            switch (currentState)
            {
                case State::NOT_PAIRED:
                case State::CONNECTING:
                case State::CHECK_MAVLINK:
                case State::UNKNOWN:
                case State::GETTING_ACTIVITIES:
                    nextMode = new Menu();
                    break;
                case State::PAIRING:
                    DOG_PRINT("[modes]{connection} stop pairing!\n");
                    BTPairing(false);
                    break;
                default:
                    DOG_PRINT("[modes]{connection} menu button not handled for this state:%d\n"
                            ,currentState);
                    break;

            }
        }
        else if (key_pressed(BTN_OK) && currentState == State::NOT_PAIRED)
        {
            // start pairing
            setState(State::PAIRING);
        }
    }
    else if (orbId == FD_MavlinkStatus && currentState == State::CHECK_MAVLINK)
    {
        int v = DataManager::instance()->mavlink_received_stats.version;

        if (v == 0)
        {
            // mavlink version not received yet
            time_t now;
            time(&now);

            if ((int)now -(int)startTime > MAVLINK_CHECK_INTERVAL)
            {
                DisplayHelper::showInfo(INFO_ERROR, MAV_VERSION_TIMEOUT);
            }
        }
        else if (v != AIRDOG_MAVLINK_VERSION)
        {
            // invalid mavlink version
            DisplayHelper::showInfo(INFO_ERROR, MAV_VERSION_MISTMATCH);
        }
        else
        {
            // valid mavlink version
            if (DataManager::instance()->activityManager.isUpdateRequired())
            {
                setState(State::GETTING_ACTIVITIES);
            }
            else
            {
                DataManager::instance()->activityManager.init();
                DataManager::instance()->activityManager.params_received();
                nextMode = new Acquiring_gps();
            }
        }
    }
    else if (orbId == FD_ActivityParams && currentState == State::GETTING_ACTIVITIES)
    {
        if (receiveActivityParams())
        {
            nextMode = new Acquiring_gps();
        }
    }

    // Check if we are in service screen
    Base* service = checkServiceScreen(orbId);
    if (service)
        nextMode = service;

    return nextMode;
}

bool ModeConnect::receiveActivityParams()
{
    bool result = false;

    result = DataManager::instance()->activityManager.params_received();

    return result;
}

void ModeConnect::getConState()
{
    DataManager *dm = DataManager::instance();
    switch(dm->bt_handler.global_state) {
        case INITIALIZING :
        case CONNECTING:
            if (currentState == State::DISCONNECTED)
            {
                break;
            }
            else 
            {
                setState(State::CONNECTING);
                break;
            }
        case NO_PAIRED_DEVICES:
            setState(State::NOT_PAIRED);
            break;
        case PAIRING:
            setState(State::PAIRING);
            break;
        case CONNECTED:
            setState(State::CHECK_MAVLINK);
            break;
        default:
            setState(State::UNKNOWN);
            break;
    }
}

void ModeConnect::BTPairing(bool start)
{
    int fd = open("/dev/btctl", 0);

    if (fd > 0) {
        if (start)
            ioctl(fd, PAIRING_ON, 0);
        else
            ioctl(fd, PAIRING_OFF, 0);
    }

    close(fd);
}

void ModeConnect::setState(State state)
{
    currentState = state;

    switch (currentState)
    {
        case State::NOT_PAIRED:
            DisplayHelper::showInfo(INFO_NOT_PAIRED);
            break;

        case State::PAIRING:
            forcing_pairing = true;
            BTPairing();
            DisplayHelper::showInfo(INFO_PAIRING);
            break;

        case State::DISCONNECTED:
            DisplayHelper::showInfo(INFO_CONNECTION_LOST);
            break;

        case State::CONNECTING:
            DisplayHelper::showInfo(INFO_CONNECTING_TO_AIRDOG);
            break;

        case State::CHECK_MAVLINK:
            time(&startTime);
            DisplayHelper::showInfo(INFO_ESTABLISHING_CONNECTION);
            break;

        case State::GETTING_ACTIVITIES:
            DisplayHelper::showInfo(INFO_GETTING_ACTIVITIES);
            DataManager::instance()->activityManager.init();
            break;

        default:
            DisplayHelper::showInfo(INFO_FAILED);
            break;
    }
}

} //end of namespace modes
