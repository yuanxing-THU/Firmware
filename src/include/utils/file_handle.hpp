#ifndef __UTILS_FILE_HANDLE_HPP_INCLUDED__
#define __UTILS_FILE_HANDLE_HPP_INCLUDED__

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>

namespace Utils {
    class File_handle {
    public:
        File_handle() : fd(-1) { }
        
        File_handle(const char * const path, const int oflags = 0, const bool verbose_failure = true) : fd(-1) {
            Open(path, oflags, verbose_failure);
        }
        
        bool Open(const char * const path, const int oflags = 0, const bool verbose_failure = true) {
            Close();
            fd = open(path, oflags);
            if ( !Is_open() ) {
                if ( verbose_failure ) printf("[Utils::File_handle] failed to open %s: %d\n", path, errno);
                return false;
            }
            return true;
        }
        
        bool Is_open() const {
            return fd >= 0;
        }
        
        void Close() {
            if ( Is_open() ) {
                close(fd);
                fd = -1;
            }
        }
        
        int Fd() {
            return fd;
        }
        
        bool IOctl(const int req, const unsigned long arg, const bool verbose_failure = true) const {
            if ( ioctl(fd, req, arg) < 0 ) {
                if ( verbose_failure ) printf("[Utils::File_handle] failed to ioctl on %d: %d/%lu", fd, req, arg);
                return false;
            }
            return true;
        }
        
        ~File_handle() {
            Close();
        }
        
    private:
        int fd;
        
    private:
        File_handle(const File_handle &);
        File_handle & operator=(const File_handle &);
    };
}

#endif
