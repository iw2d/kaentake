#pragma once 
#include "ztl/tsingleton.h"


class CConfig : public TSingleton<CConfig, 0x00BEBF9C> {
public:
    enum {
        GLOBAL_OPT = 0x0,
        LAST_CONNECT_INFO = 0x1,
        CHARACTER_OPT = 0x2,
    };

    int GetOpt_Int(int nType, const char* sKey, int nDefaultValue, int nLowBound, int nHighBound) {
        return reinterpret_cast<int(__thiscall*)(CConfig*, int, const char*, int, int, int)>(0x0049EF65)(this, nType, sKey, nDefaultValue, nLowBound, nHighBound);
    }
    void SetOpt_Int(int nType, const char* sKey, int nValue) {
        reinterpret_cast<void(__thiscall*)(CConfig*, int, const char*, int)>(0x0049EFB5)(this, nType, sKey, nValue);
    }
};