#pragma once
#include "hook.h"
#include "ztl/ztl.h"
#include <windows.h>


class CWvsApp : public TSingleton<CWvsApp, 0x00BE7B38> {
public:
    MEMBER_AT(HWND, 0x4, m_hWnd)
    MEMBER_AT(int, 0x8, m_bPCOMInitialized)
    MEMBER_AT(unsigned int, 0xC, m_dwMainThreadId)
    MEMBER_AT(HHOOK, 0x10, m_hHook)
    MEMBER_AT(int, 0x14, m_bWin9x)
    MEMBER_AT(int, 0x18, m_tUpdateTime)
    MEMBER_AT(int, 0x1C, m_bFirstUpdate)
    MEMBER_AT(ZXString<char>, 0x20, m_sCmdLine)
    MEMBER_AT(int, 0x24, m_nGameStartMode)
    MEMBER_AT(int, 0x28, m_bAutoConnect)
    MEMBER_AT(int, 0x2C, m_bShowAdBalloon)
    MEMBER_AT(int, 0x30, m_bExitByTitleEscape)
    MEMBER_AT(HRESULT, 0x34, m_hrZExceptionCode)
    MEMBER_AT(HRESULT, 0x38, m_hrComErrorCode)
    MEMBER_AT(int, 0x54, m_nTargetVersion)
    MEMBER_ARRAY_AT(void*, 0x54, m_ahInput, 3)

    // bypass.cpp
    MEMBER_HOOK(void, 0x009F4FDA, Constructor)
    MEMBER_HOOK(void, 0x009F5239, SetUp)
    MEMBER_HOOK(void, 0x009F84D0, CallUpdate, int tCurTime)
    MEMBER_HOOK(void, 0x009F5C50, Run, int* pbTerminate)

    // resman.cpp
    MEMBER_HOOK(void, 0x009F7159, InitializeResMan)
    MEMBER_HOOK(void, 0x009F69B7, CleanUp)


    static void Dir_BackSlashToSlash(char* sDir) {
        size_t uLen = strlen(sDir);
        for (size_t i = 0; i < uLen; ++i) {
            if (sDir[i] == '\\') {
                sDir[i] = '/';
            }
        }
    }
    static void Dir_SlashToBackSlash(char* sDir) {
        size_t uLen = strlen(sDir);
        for (size_t i = 0; i < uLen; ++i) {
            if (sDir[i] == '/') {
                sDir[i] = '\\';
            }
        }
    }
    static void Dir_upDir(char* sDir) {
        size_t uLen = strlen(sDir);
        if (uLen > 0 && sDir[uLen - 1] == '/') {
            sDir[uLen - 1] = 0;
        }
        for (size_t i = strlen(sDir) - 1; i > 0; --i) {
            if (sDir[i] == '/') {
                sDir[i] = 0;
                return;
            }
        }
    }
};
