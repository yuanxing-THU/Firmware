#include <nuttx/config.h>

#include "quick_log.hpp"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <systemlib/systemlib.h>
#include <utils/file_handle.hpp>

volatile bool Quick_log::s_thread_running     = false;
volatile bool Quick_log::s_thread_should_exit = false;

const char * volatile Quick_log::s_literal_buffer[Quick_log::Literal_log_buffer_size];
volatile int Quick_log::s_literal_buffer_start  = 0;
volatile int Quick_log::s_literal_buffer_length = 0;

volatile char Quick_log::s_sprintf_buffer[Quick_log::Sprintf_log_buffer_size][Quick_log::Sprintf_log_max_message_size];;
volatile int Quick_log::s_sprintf_buffer_start  = 0;
volatile int Quick_log::s_sprintf_buffer_length = 0;

bool Quick_log::Start_thread() {
    if ( s_thread_running ) {
        printf("[Quick_log] Start_thread - already running\n");
        return true;
    }
    
    s_thread_should_exit = false;
    if ( task_spawn_cmd("quick_log", SCHED_DEFAULT, 49, 1200, Thread_main, (const char **)NULL) < 0 ) {
        printf("[Quick_log] Start_thread - failed to start thread: %d\n", errno);
        return false;
    }
    
    while ( !s_thread_running && !s_thread_should_exit ) usleep(200);
    
    printf("[Quick_log] Start_thread - started\n");
    
    return true;
}

bool Quick_log::Stop_thread() {
    if ( !s_thread_running ) {
        printf("[Quick_log] Stop_thread - thread not running\n");
        return false;
    }
    
    s_thread_should_exit = true;
    while ( s_thread_running ) {
        usleep(200000);
        printf(".");
    }
    printf("[Quick_log] Stop_thread - stopped\n");
    
    return true;
}

bool Quick_log::Check_logs() {
    int current_log_index = 0;
    {
        Utils::File_handle index_file("/fs/microsd/qlog/last_index.bin", O_RDONLY, false);
        if ( index_file.Is_open() ) {
            if ( index_file.Read(&current_log_index, sizeof(current_log_index)) != sizeof(current_log_index) ) {
                current_log_index = 0;
            }
        }
    }
    
    for ( int log_num = 1; log_num <= Max_log_files; ++log_num ) {
        const int dump_index = (current_log_index + log_num) % Max_log_files;
        
        char filename_buf[32] = { 0 };
        snprintf(filename_buf, sizeof(filename_buf), "/fs/microsd/qlog/qlog_%02d.txt", dump_index);
        
        struct stat st;
        if ( stat(filename_buf, &st) == 0 ){
            const int valid_empty_log_size = strlen("[Quick_log] starting") + 1;
            const int log_size = int(st.st_size);
            if ( log_size != valid_empty_log_size ) {
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                printf("!! [Quick_log] size of %s is %d, dumping:\n!> ", filename_buf, log_size);
                Utils::File_handle log_file(filename_buf, O_RDONLY);
                char c = 0, prev_c = 0;
                if ( log_file.Is_open() ) {
                    for ( int dumped_so_far = 0; dumped_so_far < log_size; ++dumped_so_far ) {
                        if ( log_file.Read(&c, 1) != 1 ) break;
                        if ( prev_c == '\n' ) printf("!> ");
                        printf("%c", c);
                        prev_c = c;
                    }
                }
                if ( c != 0 && c != '\n' ) printf("\n");
                printf("!! [Quick_log] end of dump of %s (%d)\n", filename_buf, log_size);
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }
        }
    }
}

bool Quick_log::Clear_previous_logs() {
    int current_log_index = -1;
    
    if ( s_thread_running ) {
        Utils::File_handle index_file("/fs/microsd/qlog/last_index.bin", O_RDONLY, false);
        if ( index_file.Is_open() ) {
            if ( index_file.Read(&current_log_index, sizeof(current_log_index)) != sizeof(current_log_index) ) {
                current_log_index = -1;
            }
        }
    }
    
    for ( int log_num = 0; log_num < Max_log_files; ++log_num ) if ( log_num != current_log_index ) {
        char filename_buf[32] = { 0 };
        snprintf(filename_buf, sizeof(filename_buf), "/fs/microsd/qlog/qlog_%02d.txt", log_num);
        
        struct stat st;
        if ( stat(filename_buf, &st) == 0 ){
            unlink(filename_buf);
        }
    }
}

bool Quick_log::Clear_all_logs() {
    const bool thread_was_running = s_thread_running;
    
    if ( thread_was_running ) {
        Stop_thread();
    }
    
    for ( int log_num = 0; log_num < Max_log_files; ++log_num ) {
        char filename_buf[32] = { 0 };
        snprintf(filename_buf, sizeof(filename_buf), "/fs/microsd/qlog/qlog_%02d.txt", log_num);
        
        struct stat st;
        if ( stat(filename_buf, &st) == 0 ){
            unlink(filename_buf);
        }
    }
    
    if ( thread_was_running ) {
        Start_thread();
    }
}

void Find_and_open_log_file(Utils::File_handle & log_file, const int max_log_files);

int Quick_log::Thread_main(int argc, char *argv[]) {
    s_thread_running = true;
    
    Utils::File_handle log_file;
    
    Find_and_open_log_file(log_file, Max_log_files);
    
    if ( log_file.Is_open() ) {
        QLOG_literal("[Quick_log] starting");
        
        while ( !s_thread_should_exit ) {
            sched_lock();
            const int dump_literals_start  = s_literal_buffer_start;
            const int dump_literals_length = s_literal_buffer_length;
            const int dump_sprintfs_start  = s_sprintf_buffer_start;
            const int dump_sprintfs_length = s_sprintf_buffer_length;
            sched_unlock();
            
            for ( int literal_num = 0; literal_num < dump_literals_length; ++literal_num ) {
                const int index = (dump_literals_start + literal_num) % Literal_log_buffer_size;
                log_file.Write(s_literal_buffer[index], strlen(s_literal_buffer[index]));
                log_file.Write("\n", 1);
                printf("%s\n", s_literal_buffer[index]);
            }
            
            for ( int sprintf_num = 0; sprintf_num < dump_sprintfs_length; ++sprintf_num ) {
                const int index = (dump_sprintfs_start + sprintf_num) % Sprintf_log_buffer_size;
                log_file.Write((char*)s_sprintf_buffer[index], strlen((char*)s_sprintf_buffer[index]));
                log_file.Write("\n", 1);
                printf("%s\n", (char*)s_sprintf_buffer[index]);
            }
            
            sched_lock();
            s_literal_buffer_start    = (dump_literals_start + dump_literals_length) % Literal_log_buffer_size;
            s_literal_buffer_length  -= dump_literals_length;
            s_sprintf_buffer_start    = (dump_sprintfs_start + dump_sprintfs_length) % Sprintf_log_buffer_size;
            s_sprintf_buffer_length  -= dump_sprintfs_length;
            sched_unlock();
            
            if ( dump_literals_length > 0 || dump_sprintfs_length > 0 ) {
                log_file.Fsync();
            }
            
            usleep(Dump_to_file_interval_ms * 1000);
        }
    } else {
        printf("[Quick_log] failed to run main loop");
    }
    
    s_thread_running = false;
    return 0;
}

void Find_and_open_log_file(Utils::File_handle & log_file, const int max_log_files) {
    const int mkdir_ret = mkdir("/fs/microsd/qlog", S_IRWXU | S_IRWXG | S_IRWXO);
    if ( mkdir_ret != 0 && errno != EEXIST ) {
        printf("[Quick_log] failed to mkdir: %d\n", errno);
        return;
    }
    
    int use_index = 0;
    {
        Utils::File_handle index_file("/fs/microsd/qlog/last_index.bin", O_RDONLY, false);
        if ( index_file.Is_open() ) {
            if ( index_file.Read(&use_index, sizeof(use_index)) == sizeof(use_index) ) {
                use_index = (use_index + 1) % max_log_files;
            } else {
                use_index = 0;
            }
        }
    }
    
    char filename_buf[32] = { 0 };
    snprintf(filename_buf, sizeof(filename_buf), "/fs/microsd/qlog/qlog_%02d.txt", use_index);
    if ( !log_file.Open(filename_buf, O_WRONLY | O_CREAT | O_TRUNC) ) {
        printf("[Quick_log] failed to open %s for writing\n", filename_buf);
        return;
    }
    
    {
        Utils::File_handle index_file("/fs/microsd/qlog/last_index.bin", O_WRONLY | O_CREAT | O_TRUNC);
        if ( index_file.Is_open() ) {
            index_file.Write(&use_index, sizeof(use_index));
        }
    }
    
    {
        Utils::File_handle index_file("/fs/microsd/qlog/last_index.txt", O_WRONLY | O_CREAT | O_TRUNC);
        if ( index_file.Is_open() ) {
            index_file.Write(filename_buf, strlen(filename_buf));
            index_file.Write("\n", 1);
        }
    }
}
