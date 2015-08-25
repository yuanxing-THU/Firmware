#include <nuttx/config.h>

#include <string.h>
#include <stdio.h>

#include "quick_log.hpp"

extern "C" __EXPORT int qlog_main(int argc, char *argv[]);

int qlog_main(int argc, char *argv[]) {
    if ( argc == 2 && !strcmp(argv[1], "start") ) {
        if ( !Quick_log::Start_thread() ) {
            return 2;
        }
    } else if ( argc == 2 && !strcmp(argv[1], "stop") ) {
        if ( !Quick_log::Stop_thread() ) {
            return 3;
        }
    } else if ( argc == 2 && !strcmp(argv[1], "check") ) {
        if ( !Quick_log::Check_logs() ) {
            return 4;
        }
    } else if ( argc == 2 && !strcmp(argv[1], "clear") ) {
        if ( !Quick_log::Clear_previous_logs() ) {
            return 5;
        }
    } else if ( argc == 2 && !strcmp(argv[1], "clearall") ) {
        if ( !Quick_log::Clear_all_logs() ) {
            return 6;
        }
    } else {
        QLOG_literal("[QLOG] bad usage of qlog");
        printf("usage: qlog [start|stop|check|clear|clearall]\n");
        return 1;
    }
    
    return 0;
}
