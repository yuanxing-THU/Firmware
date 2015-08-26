#include "menu.h"

#include <cstdlib>
#include <cstring>
#include <stdio.h>

#include <uORB/topics/vehicle_command.h>

#include "main.h"
#include "connect.h"
#include "calibrate.h"
#include "acquiring_gps.h"
#include "../displayhelper.h"
#include "../uorb_functions.h"

namespace modes
{

struct Menu::Entry Menu::entries[Menu::MENUENTRY_SIZE] =
{
// -------- Top level menu
{
    // Menu::MENUENTRY_ACTIVITIES,
    MENUTYPE_ACTIVITIES,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr,
    Menu::MENUENTRY_CUSTOMIZE,
    Menu::MENUENTRY_SETTINGS,
    Menu::MENUENTRY_CUR_ACTIVITY,
    Menu::MENUENTRY_EXIT,
},
{
    // Menu::MENUENTRY_CUSTOMIZE,
    MENUTYPE_CUSTOMIZE,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_SETTINGS,
    Menu::MENUENTRY_ACTIVITIES,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_EXIT,
},
{
    // Menu::MENUENTRY_SETTINGS,
    MENUTYPE_SETTINGS,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr,
    Menu::MENUENTRY_ACTIVITIES,
    Menu::MENUENTRY_CUSTOMIZE,
    Menu::MENUENTRY_PAIRING,
    Menu::MENUENTRY_EXIT,
},

// -------- Activities list
{
    // Menu::MENUENTRY_CUR_ACTIVITY,
    MENUTYPE_SELECTED_ACTIVITY,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_SELECT,
    Menu::MENUENTRY_ACTIVITIES,
},

// -------- Activity menu
{
    // Menu::MENUENTRY_SELECT,
    MENUTYPE_SELECT,
    0,
    0,
    nullptr, // use previous preset name
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_PREVIOUS,
},

// -------- Settings menu
{
    // Menu::MENUENTRY_PAIRING,
    MENUTYPE_PAIRING,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_CALIBRATION,
    Menu::MENUENTRY_AIRDOG_CALIBRATION,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_SETTINGS,
},
{
    // Menu::MENUENTRY_CALIBRATION,
    MENUTYPE_CALIBRATION,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_AIRDOG_CALIBRATION,
    Menu::MENUENTRY_PAIRING,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_SETTINGS,
},
{
    // Menu::MENUENTRY_AIRDOG_CALIBRATION,
    MENUTYPE_CALIBRATION_AIRDOG,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_PAIRING,
    Menu::MENUENTRY_CALIBRATION,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_SETTINGS,
},

// -------- Calibration menu
{
    // Menu::MENUENTRY_COMPASS,
    MENUTYPE_COMPASS,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_ACCELS,
    Menu::MENUENTRY_GYRO,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_PREVIOUS,
},
{
    // Menu::MENUENTRY_ACCELS,
    MENUTYPE_ACCELS,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_GYRO,
    Menu::MENUENTRY_COMPASS,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_PREVIOUS,
},
{
    // Menu::MENUENTRY_GYRO,
    MENUTYPE_GYRO,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_COMPASS,
    Menu::MENUENTRY_ACCELS,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_PREVIOUS,
},

// -------- Customize menu
{
    // Menu::MENUENTRY_GENERATED,
    MENUTYPE_CUSTOM_VALUE,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_CUSTOMIZE,
},
{
    // Menu::MENUENTRY_FOLLOW,
    MENUTYPE_FOLLOW,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_LAND,
    Menu::MENUENTRY_GENERATED,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_CUSTOMIZE,
},
{
    // Menu::MENUENTRY_LAND,
    MENUTYPE_LANDING,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_SAVE,
    Menu::MENUENTRY_FOLLOW,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_CUSTOMIZE,
},
{
    // Menu::MENUENTRY_SAVE,
    MENUTYPE_SAVE,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_CANCEL,
    Menu::MENUENTRY_CANCEL,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_GENERATED,
},
{
    // Menu::MENUENTRY_CANCEL,
    MENUTYPE_CANCEL,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_SAVE,
    Menu::MENUENTRY_SAVE,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_GENERATED,
},
};

Menu::Menu() :
    current_activity(0),
    activity_param(nullptr)
{
    DataManager *dm = DataManager::instance();

    int newMode = 0;
    if (!dm->activityManager.params_received())
    {
        int modes[] = {
            MENUENTRY_SETTINGS
        };
        makeMenu(modes, sizeof(modes)/sizeof(int));
        newMode = modes[0];
        //current_activity = dm->activityManager.get_current_activity();
    }
    else
    {
        int modes[] = {
            MENUENTRY_CUSTOMIZE,
            MENUENTRY_SETTINGS,
            MENUENTRY_ACTIVITIES
        };
        makeMenu(modes, sizeof(modes)/sizeof(int));
        newMode = modes[0];

        activity_param = dm->activityManager.get_current_param();
        dm->activityManager.get_display_name(currentPresetName, sizeof(currentPresetName)/sizeof(char));
        DOG_PRINT("[menu] current activity name %s\n", currentPresetName);
    }
    calibrateMode = CALIBRATE_NONE;
    previousEntry = -1;
    switchEntry(newMode);
}

void Menu::buildActivityMenu()
{
    if (key_pressed(BTN_RIGHT))
    {
        if (++current_activity == ACTIVITY_MAX)
        {
            current_activity = 0;
        }
    }
    if (key_pressed(BTN_LEFT))
    {
        if (--current_activity == -1)
        {
            current_activity = ACTIVITY_MAX-1;
        }
    }
    const char *presetName = entries[currentEntry].text;
    DisplayHelper::showMenu(entries[currentEntry].menuButtons, entries[currentEntry].menuType,
                            0, presetName, nullptr, current_activity);
}

void Menu::buildActivityParams(bool switching_from_prev_entry)
{
    DataManager *dm = DataManager::instance();

    char c_name[19];
    char c_value[10];
    char result[LEASHDISPLAY_TEXT_SIZE];

    activity_param->get_display_name(c_name, sizeof(c_name)/sizeof(char));
    activity_param->get_display_value(c_value, sizeof(c_value)/sizeof(char));

    if (key_pressed(BTN_UP))
    {
        memset(c_value, 0, sizeof(c_value));
        activity_param->get_next_value(c_value, sizeof(c_value)/sizeof(char));
        activity_param->save_value();
        backToCustomize(false);
    }
    else if (key_pressed(BTN_DOWN))
    {
        memset(c_value, 0, sizeof(c_value));
        activity_param->get_prev_value(c_value, sizeof(c_value)/sizeof(char));
        activity_param->save_value();
        backToCustomize(false);
    }
    else if (key_pressed(BTN_RIGHT)) //Don't switch param if comming from prev entry
    {
        activity_param = dm->activityManager.get_next_visible_param();
        activity_param->get_display_value(c_value, sizeof(c_value)/sizeof(char));
        activity_param->get_display_name(c_name, sizeof(c_name)/sizeof(char));
    }
    else if (key_pressed(BTN_LEFT)) //Don't switch param if comming from prev entry
    {
        activity_param = dm->activityManager.get_prev_visible_param();
        activity_param->get_display_value(c_value, sizeof(c_value)/sizeof(char));
        activity_param->get_display_name(c_name, sizeof(c_name)/sizeof(char));
    }

    if (currentEntry == MENUENTRY_GENERATED) //If not - we switched entry with Right/Left
    {
        const char *presetName = entries[currentEntry].text;

        snprintf(result, sizeof(result), "%s\n%s", c_name, c_value);
        DisplayHelper::showMenu(entries[currentEntry].menuButtons, entries[currentEntry].menuType,
                                0, presetName, result);
    }

}

int Menu::getTimeout()
{
    return -1;
}

void Menu::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
}

Base* Menu::doEvent(int orbId)
{
    Base *nextMode = nullptr;

    printf("buttons %x \n", DataManager::instance()->kbd_handler.buttons);

    if (key_pressed(BTN_OK))
    {
        if (entries[currentEntry].ok >= 0)
        {
            // save previous menu entry only when going to level down
            // skip left right movement
            previousEntry = currentEntry;
        }

        nextMode = switchEntry(entries[currentEntry].ok);
    }
    else if (key_pressed(BTN_BACK))
    {
        nextMode = switchEntry(entries[currentEntry].back);
    }
    else if (key_pressed(BTN_RIGHT))
    {
        if (currentEntry != MENUENTRY_GENERATED &&
            currentEntry != MENUENTRY_CUR_ACTIVITY)
            nextMode = switchEntry(entries[currentEntry].next);
        else
            nextMode = makeAction();
    }
    else if (key_pressed(BTN_LEFT))
    {
        if (currentEntry != MENUENTRY_GENERATED &&
            currentEntry != MENUENTRY_CUR_ACTIVITY)
            nextMode = switchEntry(entries[currentEntry].prev);
        else
            nextMode = makeAction();
    }
    else if (key_pressed(BTN_UP))
    {
        nextMode = makeAction();
    }
    else if (key_pressed(BTN_DOWN))
    {
        nextMode = makeAction();
    }

    return nextMode;
}

Base* Menu::makeAction()
{
    DataManager *dm = DataManager::instance();
    Base *nextMode = nullptr;
    int value = 0;

    switch (currentEntry)
    {
        case MENUENTRY_SELECT:
            dm->activityManager.set_waiting_for_params();
            sendAirDogCommnad(VEHICLE_CMD_NAV_REMOTE_CMD, REMOTE_CMD_SWITCH_ACTIVITY,
                    current_activity, 0, 0, 0, 0, 0);
            nextMode = new ModeConnect(); //Connect will wait for new params
            break;

        case MENUENTRY_SAVE:
            if (dm->activityManager.save_params())
            {
                backToCustomize(true);
                nextMode = new ModeConnect(); //Connect will wait for new params
            }
            else
            {
                //[TODO:YURA] Error handling should be there
                DisplayHelper::showInfo(INFO_FAILED);
            }
            break;

        case MENUENTRY_CANCEL:
            backToCustomize(true);
            dm->activityManager.cancel_params(); 
            currentEntry = MENUENTRY_CUSTOMIZE;
            showEntry();
            break;

        case MENUENTRY_GENERATED:
            buildActivityParams();
            break;

        case MENUENTRY_CUR_ACTIVITY:
            buildActivityMenu();
            break;

        case MENUENTRY_FOLLOW:
            value = entries[MENUENTRY_FOLLOW].menuValue;
            if (key_pressed(BTN_UP))
            {
                value++;
                if (value == FOLLOW_MAX)
                {
                    value = 0;
                }
            }
            else if (key_pressed(BTN_DOWN))
            {
                value--;
                if (value < 0)
                {
                    value = FOLLOW_MAX - 1;
                }
            }

            if (value != entries[MENUENTRY_FOLLOW].menuValue)
            {
                // value was changed
                entries[MENUENTRY_FOLLOW].menuValue = value;
                showEntry();
            }
            break;

        case MENUENTRY_LAND:
            value = entries[MENUENTRY_LAND].menuValue;
            if (key_pressed(BTN_UP))
            {
                value++;
                if (value == LAND_MAX)
                {
                    value = 0;
                }
            }
            else if (key_pressed(BTN_DOWN))
            {
                value--;
                if (value < 0)
                {
                    value = LAND_MAX - 1;
                }
            }

            if (value != entries[MENUENTRY_LAND].menuValue)
            {
                // value was changed
                entries[MENUENTRY_LAND].menuValue = value;
                showEntry();
            }
            break;

        case MENUENTRY_GYRO:
        {
            if (calibrateMode == CALIBRATE_LEASH)
            {
                nextMode = new Calibrate(CalibrationDevice::LEASH_GYRO);
            }
            else if (calibrateMode == CALIBRATE_AIRDOG)
            {
                nextMode = new Calibrate(CalibrationDevice::AIRDOG_GYRO);
            }
            break;
        }

        case MENUENTRY_ACCELS:
        {
            if (calibrateMode == CALIBRATE_LEASH)
            {
                nextMode = new Calibrate(CalibrationDevice::LEASH_ACCEL);
            }
            else if (calibrateMode == CALIBRATE_AIRDOG)
            {
                nextMode = new Calibrate(CalibrationDevice::AIRDOG_ACCEL);
            }
            break;
        }

        case MENUENTRY_COMPASS:
        {
            if (calibrateMode == CALIBRATE_LEASH)
            {
                nextMode = new Calibrate(CalibrationDevice::LEASH_MAGNETOMETER);
            }
            else if (calibrateMode == CALIBRATE_AIRDOG)
            {
                nextMode = new Calibrate(CalibrationDevice::AIRDOG_MAGNETOMETER);
            }
            break;
        }

        case MENUENTRY_CALIBRATION:
            previousEntry = currentEntry;
            calibrateMode = CALIBRATE_LEASH;
            switchEntry(MENUENTRY_COMPASS);
            break;

        case MENUENTRY_AIRDOG_CALIBRATION:
            previousEntry = currentEntry;
            calibrateMode = CALIBRATE_AIRDOG;
            switchEntry(MENUENTRY_COMPASS);
            break;

        case MENUENTRY_PAIRING:
            nextMode = new ModeConnect(ModeConnect::State::PAIRING);
            break;

        case MENUENTRY_CUSTOMIZE:
            int modes[] = {
                MENUENTRY_GENERATED
            };
            makeMenu(modes, sizeof(modes)/sizeof(int));
            switchEntry(modes[0]);
            break;
    }

    return nextMode;
}

void Menu::showEntry()
{
    if (currentEntry == MENUENTRY_GENERATED)
    {
        bool switching_from_prev_entry = true;
        buildActivityParams(switching_from_prev_entry);
    }
    else if (currentEntry == MENUENTRY_CUR_ACTIVITY)
    {
        buildActivityMenu();
    }
    else
    {
        const char *presetName = entries[currentEntry].text;

        if (presetName == nullptr)
        {
            presetName = currentPresetName;
        }

        printf("presetName %s\n", presetName);
        DisplayHelper::showMenu(entries[currentEntry].menuButtons, entries[currentEntry].menuType,
                                entries[currentEntry].menuValue, presetName);
    }
}

Base* Menu::switchEntry(int newEntry)
{
    Base *nextMode = nullptr;

    if (newEntry == MENUENTRY_PREVIOUS)
    {
        switchEntry(previousEntry);
    }
    else if (newEntry == MENUENTRY_ACTION)
    {
        nextMode = makeAction();
    }
    else if (newEntry == MENUENTRY_EXIT)
    {
        nextMode = new ModeConnect();
    }
    else if (newEntry < MENUENTRY_SIZE && newEntry >= 0)
    {
        DOG_PRINT("[menu] SWITCHING TO ENTRY %d from %d\n", newEntry, currentEntry);
        currentEntry = newEntry;
        showEntry();
    }

    return nextMode;
}

void Menu::backToCustomize(bool yes)
{
    if (yes)
    {
        entries[MENUENTRY_GENERATED].back = MENUENTRY_CUSTOMIZE;
    }
    else
    {
        entries[MENUENTRY_GENERATED].back = MENUENTRY_SAVE;
    }
}

void Menu::makeMenu(int menuEntry[], int size)
{
    int first = MENUENTRY_IGNORE;
    int last = MENUENTRY_IGNORE;
    int i = 0;

    // find first menu
    for (i = 0; i < size; i++)
    {
        if (menuEntry[i] != MENUENTRY_IGNORE)
        {
            first = menuEntry[i];

            entries[first].next = first;
            entries[first].prev = first;
            break;
        }
    }

    // make a loop with entries
    last = first;
    i++;
    for (; i < size; i++)
    {
        int e = menuEntry[i];
        if (e != MENUENTRY_IGNORE)
        {
            entries[last].next = e;
            entries[first].prev = e;
            entries[e].prev = last;
            entries[e].next = first;
            last = e;
        }
    }
}

}
