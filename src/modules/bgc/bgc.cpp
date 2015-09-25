#include <nuttx/config.h>

#include <cstdio>
#include <cstring>
#include <poll.h>

#include <quick_log/quick_log.hpp>
#include <systemlib/systemlib.h>

#include "bgc.hpp"

#include "bgc_uart.hpp"
#include "bgc_uart_msg.hpp"

namespace BGC {

volatile bool BGC::s_thread_running = false;
volatile bool BGC::s_thread_should_exit = false;

int BGC::s_discovered_speed = -1;
int BGC::s_discovered_parity = -1;

bool BGC::Start_thread() {
    if ( s_thread_running ) {
        QLOG_literal("[BGC] thread already running");
        return true;
    }

    s_thread_should_exit = false;
    if ( task_spawn_cmd("bgc", SCHED_DEFAULT, SCHED_PRIORITY_DEFAULT, 2000, Thread_main, (const char **)NULL) < 0 ) {
        QLOG_sprintf("[BGC] task_spawn_cmd fail: %d", errno);
        return false;
    }

    while ( !s_thread_running && !s_thread_should_exit ) {
        usleep(200);
    }
    printf("[BGC] thread started\n");

    return true;
}

bool BGC::Stop_thread() {
    if ( !s_thread_running ) {
        QLOG_literal("[BGC] thread not running");
        return false;
    }

    s_thread_should_exit = true;
    while ( s_thread_running ) {
        usleep(200000);
        printf(".");
    }
    printf("[BGC] thread stopped\n");

    return true;
}

int BGC::Thread_main(int argc, char *argv[]) {
    s_thread_running = true;
    BGC bgc;
    if ( !bgc.Initial_setup() ) {
        QLOG_literal("[BGC] fatal error, stopping thread");
        s_thread_should_exit = true;
    } else {
        for ( ; ; ) {
            if ( !bgc.Run() ) break;
            // Run will return true if it had to quit for some reason, but wants us to Run again.
            // So we sleep for some time, and try again.
            sleep(10);
        }
    }
    s_thread_running = false;
    return 0;
}

bool BGC::Factory_check() {
    if ( s_thread_running ) {
        printf("[BGC Factory Check] stop daemon first.\n");
        return false;
    }
    BGC x;
    if ( !x.bgc_uart.Open() ) {
        printf("[BGC Factory Check] open BGC uart failed.\n");
        return false;
    }
    if ( !x.Run_setup() ) {
        printf("[BGC Factory Check] communication failed.\n");
        return false;
    }
    BGC_uart_msg out_msg;
    out_msg.Build_OUT_CMD_BOARD_INFO();
    if ( !out_msg.Is_first_byte_present() || !x.bgc_uart.Send(out_msg) ) {
        printf("[BGC Factory Check] board info command failed.\n");
        return false;
    }
    if ( out_msg.Get_IN_CMD_BOARD_INFO_Firmware_ver() < 2439 ) {
        printf("[BGC Factory Check] wrong version.\n");
        return false;
    }
    printf("[BGC Factory Check] version ok.\n");
    out_msg.Build_OUT_CMD_MOTORS_OFF();
    if ( !out_msg.Is_first_byte_present() || !x.bgc_uart.Send(out_msg) ) {
        printf("[BGC Factory Check] motors off command failed.\n");
        return false;
    }
    return true;
}

BGC::BGC()
    : frame_button_subscriber()
    , vehicle_status_subscriber()
    , bgc_uart()
    , prev_arming_state(arming_state_t(ARMING_STATE_MAX))
    , arm_bgc_motors_param("A_BGC_ARM_MOTORS")
{ }

BGC::~BGC() { }

bool BGC::Initial_setup() {
    if ( !arm_bgc_motors_param.Is_open() ) return false;

    if ( !frame_button_subscriber.Open() ) return false;
    printf("[BGC] subscribed to frame button\n");

    if ( !vehicle_status_subscriber.Open() ) return false;
    if ( !vehicle_status_subscriber.Set_interval(1000) ) return false;
    printf("[BGC] subscribed to vehicle status\n");

    if ( !bgc_uart.Open() ) return false;
    printf("[BGC] opened BGC_uart\n");

    return true;
}

bool BGC::Run() {
    if ( !Run_setup() ) return !s_thread_should_exit;

    BGC_uart_msg in_msg;
    while ( !s_thread_should_exit ) {
        const Poll_result poll_result = Poll();
        if ( poll_result & Poll_result::Error ) return false;
        if ( poll_result & Poll_result::Frame_button_ready ) {
            if ( !Process_frame_button_event() ) break;
        }
        if ( poll_result & Poll_result::Vehicle_status_ready ) {
            if ( !Update_bgc_motor_status() ) break;
        }
        if ( poll_result & Poll_result::BGC_uart_ready ) {
            if ( !bgc_uart.Recv_partial(in_msg) ) break;
            if ( in_msg.Is_fully_present() ) {
                if ( in_msg.Is_fully_valid() ) {
                    if ( in_msg.Command_id() == SBGC_CMD_CONFIRM ) {
                        DOG_PRINT("[BGC] received SBGC_CMD_CONFIRM\n");
                    } else if ( in_msg.Command_id() == SBGC_CMD_ERROR ) {
                        QLOG_literal("[BGC] received SBGC_CMD_ERROR");
                        in_msg.Dump();
                    } else if ( in_msg.Command_id() == SBGC_CMD_RESET ) {
                        QLOG_literal("[BGC] received SBGC_CMD_RESET, restarting");
                        break;
                    } else {
                        QLOG_literal("[BGC] received unexpected message");
                        in_msg.Dump();
                    }
                }
                in_msg.Reset();
            }
        }
    }

    return !s_thread_should_exit;
}

bool BGC::Run_setup() {
    if ( s_discovered_speed == -1 || s_discovered_parity == -1 ) {
        if ( !Discover_attributes() ) return false;
    } else {
        if ( !bgc_uart.Set_attributes(s_discovered_speed, s_discovered_parity) ) return false;
        bool old_attributes_valid = false;
        const int old_attribute_tries = 3;
        for ( int try_nr = 0; try_nr < old_attribute_tries; ++try_nr ) {
            if ( bgc_uart.Get_board_info() ) {
                old_attributes_valid = true;
                break;
            }
        }
        if ( old_attributes_valid ) {
            printf("[BGC] successfully used old BGC_uart attributes: speed=%d parity=%d\n", s_discovered_speed, s_discovered_parity);
        } else {
            printf("[BGC] couldn't use old BGC_uart attributes: speed=%d parity=%d\n", s_discovered_speed, s_discovered_parity);
            if ( !Discover_attributes() ) return false;
        }
    }

#if CONFIG_BOARD_REVISION >= 6
    /** On newer revisions that don't have BGC Video Tx Power enabled by default through a hardware jumper,
     *  we need to send a message to the BGC chip to enable Video Tx Power by setting a pin to 1.
     */
    int32_t do_pin = 0;
    if ( !Utils::Get_param<int32_t>(do_pin, "A_BGC_EN_VTX_POW", 0, 1) ) return false;
    if ( do_pin != 0 ) {
        if ( !Enable_video_tx_power_pin() ) return false;
    }
#endif

    return true;
}

bool BGC::Discover_attributes() {
    int speed = 0, parity = 0;
    bool attributes_discovered = false;
    const int attribute_discovery_tries = 3;
    for ( int try_nr = 0; try_nr < attribute_discovery_tries; ++try_nr ) {
        if ( s_thread_should_exit ) break;
        if ( bgc_uart.Discover_attributes(speed, parity, s_thread_should_exit) ) {
            attributes_discovered = true;
            break;
        }
    }
    if ( !attributes_discovered ) {
        QLOG_literal("[BGC] failed to discover BGC_uart attributes");
        return false;
    }
    s_discovered_speed = speed;
    s_discovered_parity = parity;
    printf("[BGC] discovered BGC_uart attributes: speed=%d parity=%d\n", speed, parity);
    return true;
}

bool BGC::Process_frame_button_event() {
    if ( !frame_button_subscriber.Read() ) return false;

/** TODO! Implement detection of the DIP-switch that enables button pass-trough. Refer to
 *  https://docs.google.com/document/d/1m2cnf1UrndAgbCF8fEZWD3Evr-s2qAcHdLWIV6tyzqg/edit?usp=sharing
 *  section "Main processor LED and buttons" for more info.
 */
#if CONFIG_BOARD_REVISION < 6
    /** Don't process frame button events on old revisions
     *  that have direct electric link frame_button -> BGC
     */
    printf("[BGC] Process_frame_button_event - skipping event\n");
    return true;
#endif

    BGC_uart_msg out_msg;
    switch ( frame_button_subscriber.Data().state ) {
        case SINGLE_CLICK: {
            DOG_PRINT("[BGC] SINGLE_CLICK -> SBGC_MENU_CMD_MOTOR_TOGGLE\n");
            out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_MOTOR_TOGGLE);
            break;
        }
        case DOUBLE_CLICK: {
            DOG_PRINT("[BGC] DOUBLE_CLICK -> SBGC_MENU_CMD_CALIB_ACC\n");
            out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_CALIB_ACC);
            break;
        }
        case TRIPLE_CLICK: {
            DOG_PRINT("[BGC] TRIPLE_CLICK -> SBGC_MENU_CMD_CALIB_GYRO\n");
            out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_CALIB_GYRO);
            break;
        }
        case LONG_KEYPRESS: {
            DOG_PRINT("[BGC] LONG_KEYPRESS -> ignore\n");
            break;
        }
        default: {
            QLOG_literal("[BGC] unknown frame button state received");
            break;
        }
    }
    if ( out_msg.Is_first_byte_present() && !bgc_uart.Send(out_msg) ) {
        QLOG_literal("[BGC] failed to send SBGC_CMD_EXECUTE_MENU");
        return false;
    }
    return true;
}

bool BGC::Update_bgc_motor_status() {
    if ( !vehicle_status_subscriber.Read() ) return false;

    // DOG_PRINT("[BGC] Update_bgc_motor_status - change: %d -> %d\n", prev_arming_state, raw_vehicle_status.arming_state);
    BGC_uart_msg out_msg;
    if ( prev_arming_state != ARMING_STATE_ARMED && vehicle_status_subscriber.Data().arming_state == ARMING_STATE_ARMED ) {
        if ( arm_bgc_motors_param.Get() != 0 ) {
            DOG_PRINT("[BGC] sending CMD_MOTORS_ON\n");
            out_msg.Build_OUT_CMD_MOTORS_ON();
        }
    } else if ( (prev_arming_state == ARMING_STATE_ARMED || prev_arming_state == ARMING_STATE_MAX)
            && vehicle_status_subscriber.Data().arming_state != ARMING_STATE_ARMED ) {
        if ( arm_bgc_motors_param.Get() != 0 ) {
            DOG_PRINT("[BGC] sending CMD_MOTORS_OFF\n");
            out_msg.Build_OUT_CMD_MOTORS_OFF();
        }
    }
    prev_arming_state = vehicle_status_subscriber.Data().arming_state;
    if ( out_msg.Is_first_byte_present() && !bgc_uart.Send(out_msg) ) {
        QLOG_literal("[BGC] failed to send CMD_MOTORS_*");
        return false;
    }
    return true;
}

bool BGC::Enable_video_tx_power_pin() {
    BGC_uart_msg out_msg;
    out_msg.Build_OUT_CMD_TRIGGER_PIN(BGC_VIDEO_TX_POWER_PIN, 1);
    if ( !bgc_uart.Send(out_msg) ) {
        QLOG_literal("[BGC] failed to send CMD_TRIGGER_PIN");
        return false;
    }
    printf("[BGC] sent CMD_TRIGGER_PIN %d/%d\n", BGC_VIDEO_TX_POWER_PIN, 1);
    return true;
}

BGC::Poll_result BGC::Poll() {
    const int timeout_ms = 1000;
    const int error_limit = 3;
    for ( int error_count = 0; error_count < error_limit; ++error_count ) {
        struct pollfd pfds[3];
        pfds[0].fd = frame_button_subscriber.Fd();   pfds[0].events = POLLIN;
        pfds[1].fd = vehicle_status_subscriber.Fd(); pfds[1].events = POLLIN;
        pfds[2].fd = bgc_uart.Fd();                  pfds[2].events = POLLIN;
        const int poll_ret = poll(pfds, 3, timeout_ms);
        if ( poll_ret < 0 ) {
            QLOG_sprintf("[BGC] poll fail: %d", errno);
        } else if ( poll_ret == 0 ) {
            return Poll_result::Timeout;
        } else {
            int result = Poll_result::Unknown;
            if ( pfds[0].revents & POLLIN ) result |= Poll_result::Frame_button_ready;
            if ( pfds[1].revents & POLLIN ) result |= Poll_result::Vehicle_status_ready;
            if ( pfds[2].revents & POLLIN ) result |= Poll_result::BGC_uart_ready;
            return Poll_result(result);
        }
    }
    QLOG_literal("[BGC] poll error limit reached");
    return Poll_result::Error;
}

}
