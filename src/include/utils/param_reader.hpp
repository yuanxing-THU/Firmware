#ifndef __UTILS_PARAM_READER_HPP_INCLUDED__
#define __UTILS_PARAM_READER_HPP_INCLUDED__

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
            if ( Is_open() ) Close();
            pt = param_find(name);
            if ( !Is_open() ) {
                if ( verbose_failure ) {
                    printf("[Utils::Param_reader] Open - failed to find %s\n", name);
                }
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
            if ( Is_open() ) Close();
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
            if ( verbose_failure ) {
                printf("[Utils] Get_param - %s out of bounds\n", name);
            }
            return false;
        }
        return true;
    }
    
    template<class ParamT, class FinalT, class LimT>
    bool Get_param(FinalT & value, Param_reader<ParamT> & param, const LimT lim_min, const LimT lim_max, const bool verbose_fail = true) {
        value = param.Get();
        if ( value < FinalT(lim_min) || value > FinalT(lim_max) ) {
            if ( verbose_fail ) {
                printf("[Utils] Get_param - out of bounds\n");
            }
            return false;
        }
        return true;
    }
}

#endif
