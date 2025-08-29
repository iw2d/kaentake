#include "zlock.h"
#include <windows.h>


struct _TEB*(__fastcall* ZFatalSection::_s_pfnTry)(volatile long*) = ZFatalSection::_TryI;

struct _TEB* __fastcall ZFatalSection::_TryM(volatile long* __formal) {
    _TEB* pTeb = NtCurrentTeb();
    long result = InterlockedCompareExchange(__formal, reinterpret_cast<long>(pTeb), 0);
    if (result) {
        if (result == reinterpret_cast<long>(pTeb)) {
            InterlockedIncrement(__formal + 1);
            return nullptr;
        }
    } else {
        InterlockedExchange(__formal + 1, 1);
    }
    return reinterpret_cast<_TEB*>(result);
}

struct _TEB* __fastcall ZFatalSection::_TryS(volatile long* __formal) {
    _TEB* pTeb = NtCurrentTeb();
    long result = InterlockedCompareExchange(__formal, reinterpret_cast<long>(pTeb), 0);
    InterlockedExchange(__formal + 1, 1);
    return reinterpret_cast<_TEB*>(result);
}

struct _TEB* __fastcall ZFatalSection::_TryI(volatile long* p) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    auto pfnTry = _TryM;
    if (si.dwNumberOfProcessors <= 1) {
        pfnTry = _TryS;
    }
    _s_pfnTry = pfnTry;
    return pfnTry(p);
}

void ZFatalSection::Lock() {
    volatile long* p = reinterpret_cast<volatile long*>(this);
    if (ZFatalSection::_s_pfnTry(p) && ZFatalSection::_s_pfnTry(p)) {
        do {
            Sleep(0);
        } while (ZFatalSection::_s_pfnTry(p));
    }
}