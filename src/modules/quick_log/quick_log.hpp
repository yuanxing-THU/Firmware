#ifndef __QUICK_LOG_HPP_INCLUDED__
#define __QUICK_LOG_HPP_INCLUDED__

#include <stdio.h>
#include <sched.h>
#include <string.h>

// A class that can be used for logging from high-priority threads, will never block.
// Collects messages in a buffer, later writes them to /fs/microsd/qlog and stdout.
// Will drop messages if too many are logged before it gets the chance to output. (Will store initial messages, drop latest.)
// Should probably be only used for error messages (that, hopefully, happen rarely/never :) ),
// not status messages that happen all the time (there will be probably too many of them, and we will drop something important).
// 
// Exposes interface through QLOG_literal and QLOG_sprintf - for shorter calls. Also, if DOG_DEBUG is enabled on the module
// calling QLOG_literal/QLOG_sprintf, will dump the message immediately (at the point of the call) to DOG_DEBUG.
//
class Quick_log {
private:
    enum { Max_log_files                = 64   };
    enum { Dump_to_file_interval_ms     = 1000 };
    enum { Literal_log_buffer_size      = 32   };
    enum { Sprintf_log_buffer_size      = 8    };
    enum { Sprintf_log_max_message_size = 81   };
    
public:
    static bool Start_thread();
    static bool Stop_thread();
    
    static bool Check_logs();
    static bool Clear_previous_logs();
    static bool Clear_all_logs();
    
    // Used for logging string literals, Quick_log will simply trust that the string will _ALWAYS_ be available,
    // so don't give it some local buffer that will become invalid after a while.
    static void Log_string_literal(const char * const string_literal) {
        sched_lock();
        if ( s_literal_buffer_length < Literal_log_buffer_size ) {
            const int index = (s_literal_buffer_start + s_literal_buffer_length) % Literal_log_buffer_size;
            s_literal_buffer[index] = string_literal;
            ++s_literal_buffer_length;
        }
        sched_unlock();
    }
    
    static void Log_sprintf(const char * const format, ...) {
        char temp_str_buf[Sprintf_log_max_message_size];
        va_list ap;
        va_start(ap, format);
        const int len = vsnprintf(temp_str_buf, Sprintf_log_max_message_size, format, ap);
        va_end(ap);
        
        sched_lock();
        if ( s_sprintf_buffer_length < Sprintf_log_buffer_size ) {
            const int index = (s_sprintf_buffer_start + s_sprintf_buffer_length) % Sprintf_log_buffer_size;
            memcpy((char*)s_sprintf_buffer[index], temp_str_buf, len+1);
            ++s_sprintf_buffer_length;
        }
        sched_unlock();
    }
    
private:
    static volatile bool s_thread_running;
    static volatile bool s_thread_should_exit;
    
    static int Thread_main(int argc, char *argv[]);
    
private:
    // A circular buffer of Literal_log_buffer_size pointers to static strings,
    // oldest at s_literal_buffer_start, length stored in s_literal_buffer_length.
    static const char * volatile s_literal_buffer[Literal_log_buffer_size];
    static volatile int s_literal_buffer_start;
    static volatile int s_literal_buffer_length;
    
    // A circular buffer of Sprintf_log_buffer_size pointers to string buffers,
    // oldest at s_sprintf_buffer_start, length stored in s_sprintf_buffer_length.
    static volatile char s_sprintf_buffer[Sprintf_log_buffer_size][Sprintf_log_max_message_size];
    static volatile int s_sprintf_buffer_start;
    static volatile int s_sprintf_buffer_length;
    
private:
    Quick_log();
    Quick_log(const Quick_log &);
    Quick_log & operator=(const Quick_log &);
    ~Quick_log();
};

#define QLOG_literal(string_literal) { DOG_PRINT(string_literal); DOG_PRINT("\n"); Quick_log::Log_string_literal(string_literal); }
#define QLOG_sprintf(...)            { DOG_PRINT(__VA_ARGS__);    DOG_PRINT("\n"); Quick_log::Log_sprintf       (__VA_ARGS__);    }

#endif
