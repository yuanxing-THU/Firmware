#ifndef __UTILS_ORB_SUBSCRIBER_WO_DATA_HPP_INCLUDED__
#define __UTILS_ORB_SUBSCRIBER_WO_DATA_HPP_INCLUDED__

#include <errno.h>
#include <stdio.h>
#include <uORB/uORB.h>

namespace Utils {
    template<const struct orb_metadata * const MetaP, class DataT>
    class ORB_subscriber_wo_data {
    public:
        typedef DataT Data;
        
    public:
        ORB_subscriber_wo_data() : fd(-1) { }
        
        ORB_subscriber_wo_data(const bool verbose_failure) : fd(-1) {
            Open(verbose_failure);
        }
        
        bool Open(const bool verbose_failure = true) {
            Close();
            fd = orb_subscribe(s_uorb_meta);
            if ( !Is_open() ) {
                if ( verbose_failure ) {
                    printf("[Utils::ORB_subscriber_wo_data] failed to orb_subscribe %s: %d\n", s_uorb_meta->o_name, errno);
                }
                return false;
            }
            return true;
        }
        
        bool Is_open() const {
            return fd >= 0;
        }
        
        void Close() {
            if ( Is_open() ) {
                orb_unsubscribe(fd);
                fd = -1;
            }
        }
        
        int Fd() const {
            return fd;
        }
        
        bool Check(bool * const updated, const bool verbose_failure = true) const {
            if ( orb_check(fd, updated) < 0 ) {
                if ( verbose_failure ) {
                    printf("[Utils::ORB_subscriber_wo_data] failed to orb_check %s: %d\n", s_uorb_meta->o_name, errno);
                }
                return false;
            }
            return true;
        }
        
        bool Read(DataT * data, const bool verbose_failure = true) const {
            if ( orb_copy(s_uorb_meta, fd, data) < 0 ) {
                if ( verbose_failure ) {
                    printf("[Utils::ORB_subscriber_wo_data] failed to orb_copy %s: %d\n", s_uorb_meta->o_name, errno);
                }
                return false;
            }
            return true;
        }
        
        bool Set_interval(const int interval_ms, const bool verbose_failure = true) const {
            if ( orb_set_interval(fd, interval_ms) < 0 ) {
                if ( verbose_failure ) {
                    printf("[Utils::ORB_subscriber_wo_data] failed to orb_set_interval %s: %d\n", s_uorb_meta->o_name, errno);
                }
                return false;
            }
            return true;
        }
        
        ~ORB_subscriber_wo_data() {
            Close();
        }
        
    private:
        int fd;
        
    private:
        constexpr static const struct orb_metadata * const s_uorb_meta = MetaP;
        
    private:
        ORB_subscriber_wo_data(const ORB_subscriber_wo_data &);
        ORB_subscriber_wo_data & operator=(const ORB_subscriber_wo_data &);
    };
}

#endif
