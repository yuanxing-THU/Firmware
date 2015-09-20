#include "screen.hpp"

#include <stdio.h>
#include <string.h>
#include <display.h>
#include <uORB/topics/leash_display.h>
#include <lib/activity/activity_config_list.hpp>

#include "block.hpp"
#include "images/images.h"
#include "font.hpp"
#include "errormessages.hpp"

struct TextInfo {
    const char *text;
    Font *font;
};

enum DrawMode
{
    DrawMode_CenterXY,
    DrawMode_LeftTop,
};

static void giveActivityParams(int activity, int *imageId, const char **text = nullptr);
static void drawText(int mode, struct TextInfo *text, int rowCount, int yc, int xc, int *yStart = nullptr)
{
    int height = 0;
    int width = 0;

    // calculate total height
    height = 0;
    for (int i = 0; i < rowCount; i++)
    {
        if (text[i].text != nullptr)
        {
            height += text[i].font->getTextHeight(text[i].text) + 2;
        }
    }

    int y = 0;

    switch (mode)
    {
        case DrawMode_CenterXY:
            y = yc - height / 2;
            break;

        case DrawMode_LeftTop:
            y = yc;
            break;
    }


    if (yStart != nullptr)
    {
        *yStart = y;
    }

    for (int i = 0; i < rowCount; i++)
    {
        if (text[i].text != nullptr)
        {
            int x = 0;

            switch (mode)
            {
                case DrawMode_CenterXY:
                    width = text[i].font->getTextWidth(text[i].text);
                    x = 64 - width / 2;
                    break;

                case DrawMode_LeftTop:
                    x = xc;
                    break;
            }


            TextBlock blockText(text[i].text, x, y, text[i].font);
            blockText.draw();
            y += text[i].font->getTextHeight(text[i].text) + 2;
        }
    }
}

void Screen::init()
{
    up_display_mcu_setup();
    display_init();
}


void Screen::showLogo()
{
    BitmapBlock blockLogo(0, 0, IMAGE_SCREENS_LOGO);
    blockLogo.draw();
}

void Screen::showMain(int mode, const char *presetName, int leashBattery, int airdogBattery,
                      int airdogMode, int followMode, int landMode,
                      int leashGPS, int airdogGPS)
{
    int width = 0;
    int cx = 0;
    TextBlock blockpresetName(presetName, 4, 4, &Font::LucideGrandeSmall);
    blockpresetName.draw();

    display_draw_line(0, 19, 127, 19);

    BitmapBlock blockBattery(108, 6, IMAGE_SCREENS_BATTERY);
    blockBattery.draw();

    BitmapBlock blockBatterySegment(110, 8, IMAGE_SCREENS_BATTERY_SEGMENT);
    if (leashBattery >=20)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=40)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=60)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=80)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }



    int airdogImageId = -1;

    switch (airdogMode)
    {
        case AIRDOGMODE_NONE:
            break;

        case AIRDOGMODE_PLAY:
            airdogImageId = IMAGE_SCREENS_PLAY;
            break;

        case AIRDOGMODE_PAUSE:
            airdogImageId = IMAGE_SCREENS_PAUSE;
            break;

    }

    int followImageId = -1;
    int landImageId = -1;

    switch (followMode)
    {
        case FOLLOW_PATH:
            followImageId = IMAGE_SCREENS_ICONS_MODES_PATH;
            break;

        case FOLLOW_ABS:
            followImageId = IMAGE_SCREENS_ICONS_MODES_FIXED;
            break;

        case FOLLOW_LINE:
            followImageId = IMAGE_SCREENS_ICONS_MODES_LINE;
            break;

        case FOLLOW_ADAPTIVE:
            followImageId = IMAGE_SCREENS_ICONS_MODES_ADAP;
            break;

        case FOLLOW_CIRCLE:
            followImageId = IMAGE_SCREENS_ICONS_MODES_CIRCLE;
            break;

        default:
            followImageId = IMAGE_SCREENS_ICONS_MODES_ERROR;
            break;

    }

    switch (landMode)
    {
        case LAND_HOME:
            landImageId = IMAGE_SCREENS_ICONS_MODES_HOME;
            break;

        case LAND_SPOT:
            landImageId = IMAGE_SCREENS_ICONS_MODES_SPOT;
            break;

        default:
            landImageId = IMAGE_SCREENS_ICONS_MODES_ERROR;
            break;
    }


    if (mode == MAINSCREEN_INFO || mode == MAINSCREEN_INFO_SUB)
    {
        // draw lines
        display_draw_line(0, 63, 127, 63);
        display_draw_line(0, 19, 0, 63);

        display_draw_line(64, 19, 64, 63);

        display_draw_line(0, 29, 64, 29); // horizontal gps line


        //Display AirLeash icon
        display_draw_line(2, 23, 2, 25);
        display_draw_line(10, 23, 10, 25);
        display_draw_line(4, 21, 8, 21);
        display_draw_line(4, 27, 8, 27);
        display_put_pixel(3, 22);
        display_put_pixel(3, 26);
        display_put_pixel(9, 22);
        display_put_pixel(9, 26);
        display_fill_rectangle(4, 23, 6, 25);

        //Display AirDog icon
        display_fill_rectangle(39, 22, 40, 26);
        display_draw_line(36, 23, 43, 23);
        display_draw_line(36, 25, 43, 25);

        display_draw_line(35, 22, 36, 22);
        display_draw_line(35, 26, 36, 26);

        display_draw_line(43, 22, 44, 22);
        display_draw_line(43, 26, 44, 26);

        display_put_pixel(35, 21);
        display_put_pixel(44, 21);
        display_put_pixel(35, 27);
        display_put_pixel(44, 27);

        //Display AirLeash GPS signal
        if (leashGPS == 1)
        {            
            display_draw_line(14, 24, 15, 24);
            display_draw_line(20, 24, 21, 24);
            display_draw_line(26, 24, 27, 24);
        }
        if (leashGPS > 1)
        {
            display_fill_rectangle(13,23,16,25);
        }
        if (leashGPS > 2)
        {
            display_fill_rectangle(19,22,22,26);
        }
        if (leashGPS > 3)
        {
            display_fill_rectangle(25,21,28,27);
        }

        //Display AirDog GPS signal
        if (airdogGPS == 1)
        {
            display_draw_line(48, 24, 49, 24);
            display_draw_line(54, 24, 55, 24);
            display_draw_line(60, 24, 61, 24);
        }
        if (airdogGPS > 1)
        {
            display_fill_rectangle(47,23,50,25);
        }
        if (airdogGPS > 2)
        {
            display_fill_rectangle(53,22,56,26);
        }
        if (airdogGPS > 3)
        {
            display_fill_rectangle(59,21,62,27);
        }

        if (mode == MAINSCREEN_INFO_SUB)
        {
            // draw joystick mode picture
            display_draw_line(127, 19, 127, 63);

            width = imageInfo[IMAGE_SCREENS_ICONS_JOYSTICK].w;
            cx = (113 + imageInfo[IMAGE_SCREENS_LND_FOL].w + 64 - width) / 2 + 1;

            BitmapBlock blockJoystick(cx, 22, IMAGE_SCREENS_ICONS_JOYSTICK);
            blockJoystick.draw();
        }
        else
        {

            // draw aidog land and follow modes
            display_draw_line(64, 41, 113, 41);

            BitmapBlock blockLndFol(113, 19, IMAGE_SCREENS_LND_FOL);
            blockLndFol.draw();

            width = imageInfo[followImageId].w;
            cx = (113 + 64 - width) / 2 + 1;

            BitmapBlock blockFollow(cx, 26, followImageId);
            blockFollow.draw();

            width = imageInfo[landImageId].w;
            cx = (113 + 64 - width) / 2 + 1;

            BitmapBlock blockLand(cx, 48, landImageId);
            blockLand.draw();
        }

        // draw airdog battery level
        char buf[10];
        sprintf(buf, "%d", airdogBattery);
        TextBlock blockAirdogBattery(buf, 4, 34, &Font::LucideGrandeBig);
        blockAirdogBattery.draw();

        width = Font::LucideGrandeBig.getTextWidth(buf);

        if (airdogBattery < 100)
        {
            if (airdogImageId == -1)
            {
                TextBlock blockProcent("%", 5 + width, 39, &Font::LucideGrandeMed);
                blockProcent.draw();
            }
            else if (airdogImageId != -1)
            {
                BitmapBlock blockAirdogMode(7 + width, 37, airdogImageId);
                blockAirdogMode.draw();
            }
        }
    }
    else if (mode == MAINSCREEN_READY_TO_TAKEOFF)
    {
        const int rowCount = 2;
        struct TextInfo text[rowCount];

        memset(text, 0, sizeof(text));

        text[0].text = "Press    to";
        text[0].font = &Font::LucideGrandeSmall;
        text[1].text = "takeoff";
        text[1].font = &Font::LucideGrandeSmall;

        int y = 0;
        int yc = 19 + (64 - 20) / 2;
        drawText(DrawMode_CenterXY, text, rowCount, yc, 0, &y);

        // draw play pause button
        int x = Font::LucideGrandeSmall.getTextWidth(text[0].text);
        x = 64 - x / 2; // center
        x += Font::LucideGrandeSmall.getTextWidth("Press");
        x += 4;
        BitmapBlock blockPlayPause(x, y + 2, IMAGE_SCREENS_PLAY_PAUSE);
        blockPlayPause.draw();
    }
    else
    {
        const int rowCount = 2;
        struct TextInfo text[rowCount];

        memset(text, 0, sizeof(text));

        switch (mode)
        {
            case MAINSCREEN_LANDING:
                text[0].text = "Landing...";
                text[0].font = &Font::LucideGrandeSmall;
                break;

            case MAINSCREEN_TAKING_OFF:
                text[0].text = "Taking off...";
                text[0].font = &Font::LucideGrandeSmall;
                break;

            case MAINSCREEN_CONFIRM_TAKEOFF:
                text[0].text = "Press OK to";
                text[0].font = &Font::LucideGrandeSmall;
                text[1].text = "confirm takeoff";
                text[1].font = &Font::LucideGrandeSmall;
                break;

            case MAINSCREEN_GOING_HOME:
                text[0].text = "Going Home...";
                text[0].font = &Font::LucideGrandeSmall;
                break;
        }

        int yc = 19 + (64 - 20) / 2;
        drawText(DrawMode_CenterXY, text, rowCount, yc, 0);
    }


    /*
    // follow
    TextBlock blockFollow("Path", 66, 24, &Font::LucideGrande16);
    blockFollow.draw();
    */


    /*
    // land
    TextBlock blockLand("HOME", 66, 46, &Font::LucideGrandeSmall);
    blockLand.draw();
    */
}

void Screen::showMenu(int buttons, int type, int value, const char *presetName, int activity, const char *customText)
{
    int imageId = -1;
    int width = 0;
    const char *text = nullptr;
    const char *valueText = nullptr;
    const char *label = nullptr;
    char buf[LEASHDISPLAY_TEXT_SIZE + 1];
    int i = 0;

    buf[LEASHDISPLAY_TEXT_SIZE] = 0;

    switch (type)
    {
        case MENUTYPE_SETTINGS:
            imageId = IMAGE_SCREENS_ICONS_SETTINGS;
            text = "Settings";
            break;

        case MENUTYPE_ACTIVITIES:
            imageId = IMAGE_SCREENS_ICONS_ACTIVITIES;
            text = "Activities";
            break;

        case MENUTYPE_SELECTED_ACTIVITY:
            giveActivityParams(activity, &imageId, &text);
            break;

        case MENUTYPE_PAIRING:
            imageId = IMAGE_SCREENS_ICONS_PAIRING;
            text = "Pairing";
            break;

        case MENUTYPE_CALIBRATION:
            imageId = IMAGE_SCREENS_ICONS_CALIBRATE_LEASH;
            text = "Calibration";
            break;

        case MENUTYPE_CALIBRATION_AIRDOG:
            imageId = IMAGE_SCREENS_ICONS_CALIBRATE_DOG;
            text = "AirDog calib";
            break;

        case MENUTYPE_COMPASS:
            imageId = IMAGE_SCREENS_ICONS_COMPASS;
            text = "Compass";
            break;

        case MENUTYPE_ACCELS:
            imageId = IMAGE_SCREENS_ICONS_ACCELS;
            text = "Accels";
            break;

        case MENUTYPE_GYRO:
            imageId = IMAGE_SCREENS_ICONS_GYRO;
            text = "Gyro";
            break;

        case MENUTYPE_SELECT:
            label = presetName;
            text = "Select";
            break;

        case MENUTYPE_CUSTOMIZE:
            giveActivityParams(activity, &imageId);
            text = "Customize";
            break;

        case MENUTYPE_ALTITUDE:
            text = "Altitude";
            sprintf(buf, "%d m", value);
            valueText = buf;
            break;

        case MENUTYPE_FOLLOW:
            text = "Follow";
            switch (value)
            {
                case FOLLOW_PATH:
                    valueText = "PATH";
                    break;

                case FOLLOW_ABS:
                    valueText = "ABS";
                    break;
            }
            break;

        case MENUTYPE_LANDING:
            text = "Landing";

            switch (value)
            {
                case LAND_HOME:
                    valueText = "HOME";
                    break;

                case LAND_SPOT:
                    valueText = "SPOT";
                    break;
            }
            break;

        case MENUTYPE_CUSTOM_VALUE:
            memcpy(buf, customText, LEASHDISPLAY_TEXT_SIZE);

            text = buf;
            valueText = "";

            // search for line end
            i = 0;
            while (buf[i] != 0 && buf[i] != '\n')
            {
                i++;
            }

            if (i < LEASHDISPLAY_TEXT_SIZE)
            {
                buf[i] = 0;
                valueText = buf + i + 1;
            }

            break;

        case MENUTYPE_SAVE:
            label = presetName;
            text = "SAVE";
            break;

        case MENUTYPE_CANCEL:
            label = presetName;
            text = "CANCEL";
            break;
    }

    int yc = 32; // y center

    if (label != nullptr)
    {
        TextBlock blockpresetName(label, 4, 4, &Font::LucideGrandeSmall);
        blockpresetName.draw();

        display_draw_line(0, 19, 127, 19);

        yc = 19 + (64 - 20) / 2;
    }


    if (MENUBUTTON_LEFT & buttons)
    {
        ImageInfo info = imageInfo[IMAGE_SCREENS_LEFT];
        BitmapBlock blockFollow(0, yc - info.h / 2, IMAGE_SCREENS_LEFT);
        blockFollow.draw();
    }

    if (MENUBUTTON_RIGHT & buttons)
    {
        ImageInfo info = imageInfo[IMAGE_SCREENS_RIGHT];
        BitmapBlock blockFollow(128 - info.w, yc - info.h / 2, IMAGE_SCREENS_RIGHT);
        blockFollow.draw();
    }

    // show image and text
    if (imageId != -1 && text != nullptr)
    {
        ImageInfo info = imageInfo[imageId];
        BitmapBlock blockImage(64 - info.w / 2, 42 - info.h, imageId);
        blockImage.draw();
        fprintf(stderr, "[screen] info.w %d info.h %d info.offset %d\n"
                ,info.w
                ,info.h
                ,info.offset);

        width = Font::LucideGrandeSmall.getTextWidth(text);
        TextBlock blockText(text, 64 - width / 2, 44, &Font::LucideGrandeSmall);
        blockText.draw();
    }
    // show only text if we have a label
    else if (label != nullptr)
    {
        Font &font = strlen(text) > 8 ? Font::LucideGrandeSmall : Font::LucideGrandeMed;
        // center text if we have label
        int y = yc - font.getTextHeight(text) / 2;

        width = font.getTextWidth(text);
        TextBlock blockText(text, 64 - width / 2, y, &font);
        blockText.draw();
    }
    // show text && value text
    else if (text != nullptr && valueText != nullptr)
    {
        width = Font::LucideGrandeSmall.getTextWidth(text);
        TextBlock blockText(text, 64 - width / 2, 10, &Font::LucideGrandeSmall);
        blockText.draw();

        width = Font::LucideGrandeMed.getTextWidth(valueText);
        TextBlock blockValueText(valueText, 64 - width / 2, yc - 2, &Font::LucideGrandeMed);
        blockValueText.draw();
    }



    (void)value;
}

void Screen::showInfo(int info, int error, int leashBattery)
{
    const int rowCount = 4;
    struct TextInfo text[rowCount];
    const char *title = nullptr;
    char buffer2[20]; // buffer for error title
    char buffer[60]; // buffer for error message


    memset(text, 0, sizeof(text));

    switch (info)
    {
        case INFO_CONNECTING_TO_AIRDOG:
            text[0].text = "Connecting";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Turn on your";
            text[1].font = &Font::LucideGrandeSmall;
            text[2].text = "AirDog";
            text[2].font = &Font::LucideGrandeSmall;
            break;

        case INFO_ESTABLISHING_CONNECTION:
            text[0].text = "Establishing";
            text[0].font = &Font::LucideGrandeSmall;
            text[1].text = "connection";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_GETTING_ACTIVITIES:
            text[0].text = "SYNCING";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "activities with";
            text[1].font = &Font::LucideGrandeTiny;
            text[2].text = "AirDog";
            text[2].font = &Font::LucideGrandeTiny;
            break;

        case INFO_CONNECTION_LOST:
            text[0].text = "Connection Lost";
            text[0].font = &Font::LucideGrandeSmall;
            text[1].text = "Move closer to";
            text[1].font = &Font::LucideGrandeTiny;
            text[2].text = "AirDog";
            text[2].font = &Font::LucideGrandeTiny;
            break;

        case INFO_TAKEOFF_FAILED:
            text[0].text = "Takeoff failed";
            text[0].font = &Font::LucideGrandeSmall;

            switch (error)
            {
                default:
                    snprintf(buffer, sizeof(buffer), "Error %d", error);
                    buffer[sizeof(buffer)-1] = 0;
                    break;
            }

            text[1].text = buffer;
            text[1].font = &Font::LucideGrandeTiny;
            text[2].text = "refer to user";
            text[2].font = &Font::LucideGrandeTiny;
            text[3].text = "manual";
            text[3].font = &Font::LucideGrandeTiny;
            break;

        case INFO_CALIBRATING_SENSORS:
            text[0].text = "Calibrate";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Sensors";
            text[1].font = &Font::LucideGrandeSmall;
            text[2].text = "Press OK";
            text[2].font = &Font::LucideGrandeSmall;
            break;

        case INFO_CALIBRATING_AIRDOG:
            text[0].text = "Calibrate";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "AirDog";
            text[1].font = &Font::LucideGrandeSmall;
            text[2].text = "Press OK";
            text[2].font = &Font::LucideGrandeSmall;
            break;

        case INFO_PAIRING:
            text[0].text = "Pairing";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Long press pairing";
            text[1].font = &Font::LucideGrandeTiny;
            text[2].text = "button on AirDog";
            text[2].font = &Font::LucideGrandeTiny;
            break;

        case INFO_NOT_PAIRED:
            text[0].text = "Not paired";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Press OK to pair";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_ACQUIRING_GPS_LEASH:
            text[0].text = "Acquiring GPS";
            text[0].font = &Font::LucideGrandeSmall;
            break;

        case INFO_ACQUIRING_GPS_AIRDOG:
            text[0].text = "AirDog";
            text[0].font = &Font::LucideGrandeSmall;
            text[1].text = "Acquiring GPS";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_CALIBRATING_HOLD_STILL:
            text[0].text = "Calibrating";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Hold still";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_SUCCESS:
            text[0].text = "SUCCESS";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Press OK";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_FAILED:
            text[0].text = "FAILED";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Press OK";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_CALIBRATING_DANCE:
            text[0].text = "Calibrating";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Do The Dance";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_NEXT_SIDE_UP:
            text[0].text = "Next Side";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "UP";
            text[1].font = &Font::LucideGrandeMed;
            text[2].text = "And hold still";
            text[2].font = &Font::LucideGrandeSmall;
            break;

        case INFO_ERROR:
            const char *errorText = getErrorMessageText(error, &title);

            if (title == nullptr)
            {
                title = "Error";
            }

            snprintf(buffer2, sizeof(buffer2), "%d %s", error, title);
            buffer2[sizeof(buffer2) - 1] = 0;

            text[0].text = buffer2;
            text[0].font = Font::LucideGrandeMed.getTextWidth(buffer2) <= 126 ?
                        &Font::LucideGrandeMed :
                        &Font::LucideGrandeSmall;

            if (errorText == nullptr)
            {
                snprintf(buffer, sizeof(buffer), "%d", error);
                buffer[sizeof(buffer)-1] = 0;
                text[1].text = buffer;
                text[1].font = &Font::LucideGrandeTiny;
                text[2].text = "refer to user";
                text[2].font = &Font::LucideGrandeTiny;
                text[3].text = "manual";
                text[3].font = &Font::LucideGrandeTiny;
            }
            else
            {
                strcpy(buffer, errorText);
                char *p = buffer;

                for (int i = 1; i <= 3; i++)
                {
                    text[i].text = p;
                    text[i].font = &Font::LucideGrandeTiny;

                    // search for line end
                    while (*p != 0 && *p != '\n')
                    {
                        p++;
                    }

                    if (*p == 0)
                    {
                        // message end
                        break;
                    }
                    else
                    {
                        // we found \n
                        *p = 0;
                        p++;
                    }
                }
            }

            break;
    }

    drawText(DrawMode_CenterXY, text, rowCount, 36, 0);

    BitmapBlock blockBattery(108, 0, IMAGE_SCREENS_BATTERY);
    blockBattery.draw();

    BitmapBlock blockBatterySegment(110, 2, IMAGE_SCREENS_BATTERY_SEGMENT);
    if (leashBattery >=20)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=40)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=60)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=80)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
}

void Screen::showList(LeashDisplay_Lines lines, int lineCount)
{
    struct TextInfo text[lineCount];

    memset(text, 0, sizeof(text));

    for (int i = 0; i < lineCount; i++)
    {
        text[i].text = &lines[i][0];
        if (text[i].text[0] == 0)
        {
            // make " " string if string is empty
            // otherwise drawText will skip it
            text[i].text = " ";
        }
        text[i].font = &Font::LucideGrandeTiny;
    }

    drawText(DrawMode_LeftTop, text, lineCount, 5, 1);
}

/*
 * author:  Max
 * descr:   modifies imageId and activity text to fit current activity
 */
static void giveActivityParams(int activity, int *imageId, const char **text)
{
    switch(activity)
    {
        case ACTIVITY_SKI:
            *imageId = IMAGE_SCREENS_ICONS_SKI;
            *text = activityNames[ACTIVITY_SKI];
            break;
        case ACTIVITY_SNOWBOARD:
            *imageId = IMAGE_SCREENS_ICONS_SNOW;
            *text = activityNames[ACTIVITY_SNOWBOARD];
            break;
        case ACTIVITY_SURF:
            *imageId = IMAGE_SCREENS_ICONS_SURF;
            *text = activityNames[ACTIVITY_SURF];
            break;
        case ACTIVITY_CUSTOM:
            *imageId = IMAGE_SCREENS_ICONS_CUSTOM;
            *text = activityNames[ACTIVITY_CUSTOM];
            break;
        case ACTIVITY_TEST:
            *imageId = IMAGE_SCREENS_ICONS_TEST;
            *text = activityNames[ACTIVITY_TEST];
            break;
        case ACTIVITY_WAKE:
            *imageId = IMAGE_SCREENS_ICONS_WAKE;
            *text = activityNames[ACTIVITY_WAKE];
            break;
        case ACTIVITY_BIKE:
            *imageId = IMAGE_SCREENS_ICONS_BIKE;
            *text = activityNames[ACTIVITY_BIKE];
            break;
        case ACTIVITY_CYCLE:
            *imageId = IMAGE_SCREENS_ICONS_CYCLE;
            *text = activityNames[ACTIVITY_CYCLE];
            break;
        case ACTIVITY_SKATE:
            *imageId = IMAGE_SCREENS_ICONS_SKATE;
            *text = activityNames[ACTIVITY_SKATE];
            break;
        case ACTIVITY_KITE:
            *imageId = IMAGE_SCREENS_ICONS_KITE;
            *text = activityNames[ACTIVITY_KITE];
            break;
    }
}
