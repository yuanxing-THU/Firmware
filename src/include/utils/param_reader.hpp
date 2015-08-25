#ifndef __UTILS_PARAM_READER_HPP_INCLUDED__
#define __UTILS_PARAM_READER_HPP_INCLUDED__

#include <quick_log/quick_log.hpp>
#include <stdio.h>
#include <systemlib/param/param.h>

namespace Utils {
    template<class ParamT>
    class Param_reader {
    public:
        Param_reader() : pt(PARAM_INVALID) { }
        
        Param_reader(const char * const name, const bool verbose_failure = true) : pt(PARAM_INVALID) {
            Open(name, verbose_failure);
        }
        
        bool Open(const char * const name, const bool verbose_failure = true) {
            Close();
            pt = param_find(name);
            if ( !Is_open() ) {
                if ( verbose_failure ) QLOG_sprintf("[Param_reader] find failed %s", name);
                return false;
            }
            return true;
        }
        
        bool Is_open() const {
            return pt != PARAM_INVALID;
        }
        
        void Close() {
            pt = PARAM_INVALID; // No need to actually close something.
        }
        
        param_t Pt() {
            return pt;
        }
        
        ParamT Get() const {
            ParamT value;
            param_get(pt, &value);
            return value;
        }
        
        void Get(ParamT * value) {
            param_get(pt, value);
        }
        
        ~Param_reader() {
            Close();
        }
        
    private:
        param_t pt;
        
    private:
        Param_reader(const Param_reader &);
        Param_reader & operator=(const Param_reader &);
    };
    
    template<class ParamT, class FinalT, class LimT>
    bool Get_param(FinalT & value, const char * const name, const LimT lim_min, const LimT lim_max, const bool verbose_failure = true) {
        Param_reader<ParamT> param(name, verbose_failure);
        if ( !param.Is_open() ) return false;
        value = param.Get();
        if ( value < FinalT(lim_min) || value > FinalT(lim_max) ) {
            if ( verbose_failure ) QLOG_sprintf("[Param_reader] %s out of bounds", name);
            return false;
        }
        return true;
    }
    
    template<class ParamT, class FinalT, class LimT>
    bool Get_param(FinalT & value, Param_reader<ParamT> & param, const LimT l_min, const LimT l_max, const bool verbose_failure = true) {
        value = param.Get();
        if ( value < FinalT(l_min) || value > FinalT(l_max) ) {
            const char * const name = param_name(param.Pt());
            if ( verbose_failure ) QLOG_sprintf("[Param_reader] %s out of bounds", (name != NULL ? name : "BAD_PARAM"));
            return false;
        }
        return true;
    }
}

#endif
