#pragma once
#include "debug.h"

#ifdef _DEBUG
#define ATTACH_HOOK(TARGET, DETOUR) \
    AttachHook(reinterpret_cast<void**>(&TARGET), CastHook(&DETOUR)) ? true : (ErrorMessage("Failed to attach detour function \"%s\" at target address : 0x%08X.", #DETOUR, TARGET), false)
#else
#define ATTACH_HOOK(TARGET, DETOUR) \
    AttachHook(reinterpret_cast<void**>(&TARGET), CastHook(&DETOUR))
#endif

#define MEMBER_AT(T, OFFSET, NAME) \
    __declspec(property(get = get_##NAME, put = set_##NAME)) T NAME; \
    __forceinline const T& get_##NAME() const { \
        return *reinterpret_cast<const T*>(reinterpret_cast<void*>(this) + OFFSET); \
    } \
    __forceinline T& get_##NAME() { \
        return *reinterpret_cast<T*>(reinterpret_cast<void*>(this) + OFFSET); \
    } \
    __forceinline void set_##NAME(const T& value) { \
        *reinterpret_cast<T*>(reinterpret_cast<void*>(this) + OFFSET) = const_cast<T&>(value); \
    } \
    __forceinline void set_##NAME(T& value) { \
        *reinterpret_cast<T*>(reinterpret_cast<void*>(this) + OFFSET) = value; \
    }

#define MEMBER_ARRAY_AT(T, OFFSET, NAME, N) \
    __declspec(property(get = get_##NAME)) T(&NAME)[N]; \
    __forceinline T(&get_##NAME())[N] { \
        return *reinterpret_cast<T(*)[N]>(reinterpret_cast<void*>(this) + OFFSET); \
    }


// called in injector.cpp -> DllMain
void AttachSystemHooks();

// called in system.cpp -> CreateMutexA_hook
void AttachResolutionMod();

inline void AttachClientHooks() {
    AttachResolutionMod();
}


template <typename T>
__forceinline auto CastHook(T fn) -> void* {
    union {
        T fn;
        void* p;
    } u;
    u.fn = fn;
    return u.p;
}

bool AttachHook(void** ppTarget, void* pDetour);

void* VMTHook(void* pInstance, void* pDetour, size_t uIndex);

void* GetAddress(const char* sModuleName, const char* sProcName);

void* GetAddressByPattern(const char* sModuleName, const char* sPattern);

void PatchMemory(void* pAddress, void* pValue, size_t uSize);

template <typename T>
void Patch1(T pAddress, unsigned char uValue) {
    PatchMemory(reinterpret_cast<void*>(pAddress), &uValue, sizeof(uValue));
}

template <typename T>
void Patch4(T pAddress, unsigned int uValue) {
    PatchMemory(reinterpret_cast<void*>(pAddress), &uValue, sizeof(uValue));
}

template <typename T>
void PatchStr(T uAddress, const char* sValue) {
    PatchMemory(reinterpret_cast<void*>(pAddress), &uValue, strlen(sValue));
}

template <typename T>
void PatchJmp(T pAddress, void* pDestination) {
    Patch1(pAddress, 0xE9);
    Patch4(pAddress + 1, reinterpret_cast<uintptr_t>(pDestination) - reinterpret_cast<uintptr_5>(pAddress) - 5);
}

template <typename T>
void PatchCall(T pAddress, void* pDestination) {
    Patch1(pAddress, 0xE8);
    Patch4(pAddress + 1, reinterpret_cast<uintptr_t>(pDestination) - reinterpret_cast<uintptr_5>(pAddress) - 5);
}

template <typename T>
void PatchRetZero(T pAddress) {
    PatchStr(pAddress, "\x33\xC0\xC3");
}