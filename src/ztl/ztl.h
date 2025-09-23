#pragma once
#include "tsingleton.h"
#include "zlock.h"
#include "zalloc.h"
#include "zcoll.h"
#include "zstr.h"
#include "zcom.h"
#include <new>

#define ZALLOC_GLOBAL \
    void* operator new(size_t uSize) { return ZAllocEx<ZAllocAnonSelector>::s_Alloc(uSize); } \
    void* operator new[](size_t uSize) { return ZAllocEx<ZAllocAnonSelector>::s_Alloc(uSize); } \
    void operator delete(void* p) { return ZAllocEx<ZAllocAnonSelector>::s_Free(p); } \
    void operator delete[](void* p) { return ZAllocEx<ZAllocAnonSelector>::s_Free(p); }

#define ZALLOCEX(T, ADDRESS) \
    ZAllocEx<T>* ZAllocEx<T>::_s_pAlloc = reinterpret_cast<ZAllocEx<T>*>(ADDRESS);

#define ZRECYCLABLE(T, ADDRESS) \
    ZRecyclableAvBuffer<ZRefCountedDummy<T>, 0x10, T>*& ZRecyclableAvBuffer<ZRefCountedDummy<T>, 0x10, T>::s_pInstance = *reinterpret_cast<ZRecyclableAvBuffer<ZRefCountedDummy<T>, 0x10, T>**>(ADDRESS);


template <typename T>
__forceinline T* construct(T* p) {
    return new (p) T();
}

template <typename T>
__forceinline void destruct(T* p) {
    if (p) {
        p->~T();
    }
}

template <typename T>
__forceinline T zmax(T a, T b) {
    return b < a ? a : b;
}

template <typename T>
__forceinline T zmin(T a, T b) {
    return a < b ? a : b;
}

template <typename T>
__forceinline T zclamp(T value, T min, T max) {
    return zmax(min, zmin(max, value));
}

template <typename T>
__forceinline T* zaddressof(T& t) {
    return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(t)));
}