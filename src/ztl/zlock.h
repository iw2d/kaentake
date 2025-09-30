#pragma once
#include <windows.h>

class ZFatalSectionData {
public:
    void* _m_pTIB = nullptr;
    volatile long _m_nRef = 0;
};

static_assert(sizeof(ZFatalSectionData) == 0x8);


class ZFatalSection : public ZFatalSectionData {
private:
    typedef struct _TEB*(__fastcall* pfnTry_t)(volatile long*);
    static pfnTry_t _s_pfnTry;

    static struct _TEB* __fastcall _TryM(volatile long* __formal) {
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
    static struct _TEB* __fastcall _TryS(volatile long* __formal) {
        _TEB* pTeb = NtCurrentTeb();
        long result = InterlockedCompareExchange(__formal, reinterpret_cast<long>(pTeb), 0);
        InterlockedExchange(__formal + 1, 1);
        return reinterpret_cast<_TEB*>(result);
    }
    static struct _TEB* __fastcall _TryI(volatile long* p) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        pfnTry_t pfnTry = _TryM;
        if (si.dwNumberOfProcessors <= 1) {
            pfnTry = _TryS;
        }
        _s_pfnTry = pfnTry;
        return pfnTry(p);
    }

public:
    void Lock() {
        volatile long* p = reinterpret_cast<volatile long*>(this);
        if (ZFatalSection::_s_pfnTry(p) && ZFatalSection::_s_pfnTry(p)) {
            do {
                Sleep(0);
            } while (ZFatalSection::_s_pfnTry(p));
        }
    }
};
inline ZFatalSection::pfnTry_t ZFatalSection::_s_pfnTry = ZFatalSection::_TryI;
static_assert(sizeof(ZFatalSection) == 0x8);


template <typename T>
class ZSynchronizedHelper;

template <>
class ZSynchronizedHelper<ZFatalSection> {
public:
    ZFatalSection* m_pLock;

    ZSynchronizedHelper() = delete;
    explicit ZSynchronizedHelper(ZFatalSection& lock) {
        m_pLock = &lock;
        lock.Lock();
    }
    explicit ZSynchronizedHelper(ZFatalSection* lock) {
        m_pLock = lock;
        lock->Lock();
    }
    ~ZSynchronizedHelper() {
        if (m_pLock->_m_nRef-- == 1) {
            m_pLock->_m_pTIB = nullptr;
        }
    }
};
static_assert(sizeof(ZSynchronizedHelper<ZFatalSection>) == 0x4);