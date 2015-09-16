#include "errormessages.hpp"

const char *getErrorMessageText(int errorCode, const char **pTitle)
{
    const char *result = nullptr;
    const char *title = nullptr;

    switch (errorCode)
    {
        case COMMANDER_ERROR_OK:
            result = "OK";
            break;
        case COMMANDER_ERROR_NOT_ACTIVATED:
            title = "ATTENTION";
            result = "Please activate\nyour Airdog\nvia mobile app";
            break;

        case PMC_ERROR:
            title = "ERROR";
            result = "Motor faulty";
            break;

        case PMC_ERROR_INITIAL_TILT:
            title = "CHECK";
            result = "AirDog tilted\nFind a flat place\nfor takeoff";
            break;

        case PMC_ERROR_TOO_MUCH_VIBRATION:
            title = "VIBRATIONS";
            result = "Check for grass,\nsnow or replace\ndamaged props";
            break;

        case PMC_ERROR_TOO_MUCH_TILT:
            title = "TILTING";
            result = "Check props\nand direction";
            break;

        case FTC_ERROR:
            title = "ERROR";
            result = "General failure";
            break;

        case FTC_ERROR_TAKEOFF_TOO_MUCH_TILT:
            title = "TILTING";
            result = "Check props\nand direction";
            break;

        case FTC_ERROR_TAKEOFF_NO_ALTITUDE_GAIN:
            title = "ERROR";
            result = "Takeoff altitude\ncould not be\nreached";
            break;

        case FTC_ERROR_FLIGHT_TOO_MUCH_TILT:
            title = "TILTING";
            result = "AirDog was killed\ndue to ecxessive\ntilting";
            break;

        case FTC_ERROR_LANDING_TOO_MUCH_TILT:
            title = "TILTING";
            result = "Too much tilting\nat landing";
            break;

        case SAH_ERROR:
            title = "SAFETY";
            result = "General failure";
            break;

        case BSC_ERROR:
            title = "BATTERY";
            result = "General failure";
            break;

        case BSC_ERROR_BATTERY_RTH_NOTIFY:
            title = "WARNING";
            result = "Return to HOME\nadvised";
            break;

        case BSC_ERROR_BATTERY_RTH:
            title = "BATTERY";
            result = "Returning to HOME";
            break;

        case BSC_ERROR_BATTERY_RTH_WO_BATT:
            title = "BATTERY";
            result = " Emergancy return\nto HOME";
            break;

        case BSC_ERROR_BATTERY_SWITCH_RTH_TO_LAND:
            title = "BATTERY";
            result = "Unexpected battery\ndrop, swithing to\nland mode";
            break;

        case BSC_ERROR_BATTERY_LAND_NOTIFY:
            title = "BATTERY";
            result = "Battery low\nplease land";
            break;

        case BSC_ERROR_BATTERY_LAND:
            title = "BATTERY";
            result = "Auto landing";
            break;

        case BSC_ERROR_BATTERY_LAND_DEATH:
            title = "BATTERY";
            result = "Emergency landing";
            break;
    }

    if (pTitle != nullptr)
    {
        *pTitle = title;
    }

    return result;
}
