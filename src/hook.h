#pragma once
#include "debug.h"
#include <type_traits>

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
        return *reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(this) + OFFSET); \
    } \
    __forceinline T& get_##NAME() { \
        return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + OFFSET); \
    } \
    __forceinline void set_##NAME(const T& value) { \
        *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + OFFSET) = const_cast<T&>(value); \
    } \
    __forceinline void set_##NAME(T& value) { \
        *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + OFFSET) = value; \
    }

#define MEMBER_ARRAY_AT(T, OFFSET, NAME, N) \
    __declspec(property(get = get_##NAME)) T(&NAME)[N]; \
    __forceinline T(&get_##NAME())[N] { \
        return *reinterpret_cast<T(*)[N]>(reinterpret_cast<uintptr_t>(this) + OFFSET); \
    }

#define MEMBER_HOOK(T, ADDRESS, NAME, ...) \
    inline static auto NAME = reinterpret_cast<T(__thiscall*)(void*, __VA_ARGS__)>(ADDRESS); \
    T NAME##_hook(__VA_ARGS__);

#define TO_UINTPTR(VALUE) ((uintptr_t)(VALUE))

#define TO_PVOID(VALUE) ((void*)(VALUE))


// called in injector.cpp -> DllMain
void AttachSystemHooks();

// called in system.cpp -> CreateMutexA_hook
void AttachClientBypass();
void AttachResManMod();
void AttachResolutionMod();

inline void AttachClientHooks() {
    AttachClientBypass();
    AttachResManMod();
    AttachResolutionMod();
}


template <typename T>
constexpr auto CastHook(T fn) -> void* {
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
    PatchMemory(TO_PVOID(pAddress), &uValue, sizeof(uValue));
}

template <typename T>
void Patch4(T pAddress, unsigned int uValue) {
    PatchMemory(TO_PVOID(pAddress), &uValue, sizeof(uValue));
}

template <typename T>
void PatchStr(T pAddress, const char* sValue) {
    PatchMemory(TO_PVOID(pAddress), TO_PVOID(sValue), strlen(sValue));
}

template <typename T, typename U>
void PatchNop(T pAddress, U pDestination) {
    size_t uSize = TO_UINTPTR(pDestination) - TO_UINTPTR(pAddress);
    void* pValue = malloc(uSize);
    memset(pValue, 0x90, uSize);
    PatchMemory(TO_PVOID(pAddress), pValue, uSize);
    free(pValue);
}

template <typename T, typename U>
void PatchJmp(T pAddress, U pDestination) {
    Patch1(pAddress, 0xE9);
    Patch4(pAddress + 1, TO_UINTPTR(pDestination) - TO_UINTPTR(pAddress) - 5);
}

template <typename T, typename U>
void PatchCall(T pAddress, U pDestination, size_t uSize = 5) {
    if (uSize < 5) {
        ErrorMessage("Cannot PatchCall at 0x%08X with uSize = %d", TO_UINTPTR(pAddress), uSize);
        return;
    }
    Patch1(pAddress, 0xE8);
    Patch4(pAddress + 1, TO_UINTPTR(pDestination) - TO_UINTPTR(pAddress) - 5);
    if (uSize > 5) {
        PatchNop(pAddress + 5, pAddress + uSize);
    }
}

template <typename T>
void PatchRetZero(T pAddress) {
    PatchStr(pAddress, "\x33\xC0\xC3");
}