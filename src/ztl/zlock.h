#pragma once


struct _TEB;


class ZFatalSectionData {
public:
    void* _m_pTIB = nullptr;
    volatile long _m_nRef = 0;
};
static_assert(sizeof(ZFatalSectionData) == 0x8);


class ZFatalSection : public ZFatalSectionData {
private:
    static struct _TEB*(__fastcall* _s_pfnTry)(volatile long*);
    static struct _TEB* __fastcall _TryM(volatile long* __formal);
    static struct _TEB* __fastcall _TryS(volatile long* __formal);
    static struct _TEB* __fastcall _TryI(volatile long* p);

public:
    void Lock();
};
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