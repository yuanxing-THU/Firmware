#pragma once

#include "base.h"

#include <uORB/topics/leash_display.h>
#include <activity/activity_change_manager.hpp>

namespace modes
{

class Menu : public Base
{
public:
    Menu();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);

protected:
    enum {
        // functional entries
        MENUENTRY_ACTION = - 4,
        MENUENTRY_PREVIOUS = -3,
        MENUENTRY_IGNORE = -2,
        MENUENTRY_EXIT = -1,
        // Menu entries

        // Top level menu
        MENUENTRY_ACTIVITIES,
        MENUENTRY_CUSTOMIZE,
        MENUENTRY_SETTINGS,

        // Activities list
        MENUENTRY_SNOWBOARD,
        MENUENTRY_SURF,

        // Activity menu
        MENUENTRY_SELECT,

        // Settings menu
        MENUENTRY_PAIRING,
        MENUENTRY_CALIBRATION,
        MENUENTRY_AIRDOG_CALIBRATION,

        // Calibration menu
        MENUENTRY_COMPASS,
        MENUENTRY_ACCELS,
        MENUENTRY_GYRO,

        // Customize menu
        MENUENTRY_GENERATED,
        MENUENTRY_FOLLOW,
        MENUENTRY_LAND,
        MENUENTRY_SAVE,
        MENUENTRY_CANCEL,

        // Total menu entries count
        MENUENTRY_SIZE
    };

    struct Entry
    {
        int menuType;
        int menuValue;
        int menuButtons;
        const char *text;
        int next;
        int prev;
        int ok;
        int back;
    };

    enum {
        CALIBRATE_NONE,
        CALIBRATE_LEASH,
        CALIBRATE_AIRDOG,
    };

    int calibrateMode;
    int currentEntry;
    int previousEntry;
    char currentPresetName[20];
    static struct Entry entries[MENUENTRY_SIZE];

    Base* makeAction();
    void showEntry();
    Base* switchEntry(int newEntry);

    void makeMenu(int menuEntry[], int size);
    void buildActivityMenu(bool switching_from_prev_entry = false);
    Activity::ParamChangeManager * activity_param;
    void backToCustomize(bool);
};

}
