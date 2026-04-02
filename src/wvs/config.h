#pragma once
#include "hook.h"
#include "ztl/ztl.h"


class CConfig : public TSingleton<CConfig, 0x00BEBF9C> {
public:
    enum {
        GLOBAL_OPT = 0x0,
        LAST_CONNECT_INFO = 0x1,
        CHARACTER_OPT = 0x2,
    };
    MEMBER_ARRAY_AT(int, 0xCC, m_nUIWnd_X, 34)
    MEMBER_ARRAY_AT(int, 0x154, m_nUIWnd_Y, 34)

    // resolution.cpp
    MEMBER_HOOK(void, 0x0049F0B5, GetUIWndPos, int nUIType, int* x, int* y, int* op)
    MEMBER_HOOK(void, 0x0049D0B6, LoadCharacter, int nWorldID, unsigned int dwCharacterId)
    MEMBER_HOOK(void, 0x0049C441, LoadGlobal)
    MEMBER_HOOK(void, 0x0049C8E7, SaveGlobal)
    MEMBER_HOOK(void, 0x0049EA33, ApplySysOpt, void* pSysOpt, int bApplyVideo)

    int GetOpt_Int(int nType, const char* sKey, int nDefaultX, int nLowBound, int nHighBound) {
        return reinterpret_cast<int(__thiscall*)(CConfig*, int, const char*, int, int, int)>(0x0049EF65)(this, nType, sKey, nDefaultX, nLowBound, nHighBound);
    }
    void SetOpt_Int(int nType, const char* sKey, int nValue) {
        reinterpret_cast<void(__thiscall*)(CConfig*, int, const char*, int)>(0x0049EFB5)(this, nType, sKey, nValue);
    }
};