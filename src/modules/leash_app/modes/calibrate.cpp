#include "calibrate.h"

#include <stdio.h>
#include <unistd.h>

#include <systemlib/systemlib.h>

#include "main.h"
#include "../datamanager.h"
#include "../displayhelper.h"
#include "../../airdog/calibrator/calibrator.hpp"
#include "../../airdog/calibrator/calibration_commons.hpp"

namespace modes
{

static int calibration_task;

static int start_calibrate_accelerometer(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    sleep(1);
    calibration::calibrate_accelerometer();
    return 0;
}

Calibrate::Calibrate(CalibrationDevice pDevice) :
    device(pDevice)
{
    switch (device)
    {
        case CalibrationDevice::LEASH_ACCEL:
            calibration_task = task_spawn_cmd("leash_app",
                                              SCHED_DEFAULT,
                                              SCHED_PRIORITY_DEFAULT - 30,
                                              3000,
                                              start_calibrate_accelerometer,
                                              nullptr);
            DisplayHelper::showInfo(INFO_NEXT_SIDE_UP, 0);
            break;

        case CalibrationDevice::LEASH_GYRO:
            break;
    }
}

int Calibrate::getTimeout()
{
    return -1;
}

void Calibrate::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
    awaitMask[FD_Calibrator] = 1;
}

Base* Calibrate::doEvent(int orbId)
{
    Base *nextMode = nullptr;

    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_BACK))
        {
            calibration::calibrate_stop();
            nextMode = new Main();
        }
        else if (key_pressed(BTN_OK) &&
                 DataManager::instance()->calibrator.status == CALIBRATOR_FINISH)
        {
            calibration::calibrate_stop();
            nextMode = new Main();
        }
    }
    else if (orbId == FD_Calibrator)
    {
        int status = DataManager::instance()->calibrator.status;

        switch (status)
        {
            case CALIBRATOR_DETECTING_SIDE:
                DisplayHelper::showInfo(INFO_NEXT_SIDE_UP, 0);
                break;

            case CALIBRATOR_CALIBRATING:
                DisplayHelper::showInfo(INFO_CALIBRATING_HOLD_STILL, 0);
                break;

            case CALIBRATOR_FINISH:
                calibration::CALIBRATION_RESULT result = DataManager::instance()->calibrator.result;
                if (result == calibration::CALIBRATION_RESULT::SUCCESS)
                {
                    DisplayHelper::showInfo(INFO_SUCCESS, 0);
                }
                else
                {
                    DisplayHelper::showInfo(INFO_FAILED, 0);
                }
                break;
        }
    }

    return nextMode;
}

}