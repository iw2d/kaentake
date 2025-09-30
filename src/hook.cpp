#include "pch.h"
#include "hook.h"
#include "debug.h"
#include <windows.h>
#include <detours.h>
#include <psapi.h>


namespace {

bool HexCharToByte(char c, unsigned char* b) {
    if ('0' <= c && c <= '9')
        *b = c - '0';
    else if ('A' <= c && c <= 'F')
        *b = 10 + (c - 'A');
    else if ('a' <= c && c <= 'f')
        *b = 10 + (c - 'a');
    else
        return false;
    return true;
}

size_t ParsePattern(const char* sPattern, unsigned char* abPattern, unsigned char* abMask) {
    size_t i = 0;
    while (*sPattern) {
        if (*sPattern == ' ') {
            sPattern++;
            continue;
        }
        if (sPattern[0] == '?' && sPattern[1] == '?') {
            abMask[i] = 0x00;
        } else {
            unsigned char high, low;
            if (!HexCharToByte(sPattern[0], &high) || !HexCharToByte(sPattern[1], &low)) {
                return 0;
            }
            abPattern[i] = (high << 4) | low;
            abMask[i] = 0xFF;
        }
        sPattern += 2;
        i += 1;
    }
    return i;
}

void* FindPattern(unsigned char* pModuleBase, size_t uModuleSize, unsigned char* abPattern, unsigned char* abMask, size_t uPatternSize) {
    for (size_t i = 0; i <= uModuleSize - uPatternSize; ++i) {
        size_t j;
        for (j = 0; j < uPatternSize; ++j) {
            if ((pModuleBase[i + j] & abMask[j]) != (abPattern[j] & abMask[j])) {
                break;
            }
        }
        if (j == uPatternSize) {
            return &pModuleBase[i];
        }
    }
    return nullptr;
}

} // namespace


bool AttachHook(void** ppTarget, void* pDetour) {
    LONG result;
    if (result = DetourTransactionBegin(); result != NO_ERROR) {
        DEBUG_MESSAGE("DetourTransactionBegin failed with : %d", result);
        return false;
    }
    if (result = DetourUpdateThread(GetCurrentThread()); result != NO_ERROR) {
        DEBUG_MESSAGE("DetourUpdateThread failed with : %d", result);
        DetourTransactionAbort();
        return false;
    }
    if (result = DetourAttach(ppTarget, pDetour); result != NO_ERROR) {
        DEBUG_MESSAGE("DetourAttach failed with : %d", result);
        DetourTransactionAbort();
        return false;
    }
    if (result = DetourTransactionCommit(); result != NO_ERROR) {
        DEBUG_MESSAGE("DetourTransactionCommit failed with : %d", result);
        DetourTransactionAbort();
        return false;
    }
    return true;
}

void* VMTHook(void* pInstance, void* pDetour, size_t uIndex) {
    void** vtable = *static_cast<void***>(pInstance);
    void* pTarget = vtable[uIndex];
    AttachHook(&pTarget, pDetour);
    return pTarget;
}

void* GetAddress(const char* sModuleName, const char* sProcName) {
    HMODULE hModule = GetModuleHandleA(sModuleName);
    if (!hModule) {
        hModule = LoadLibraryA(sModuleName);
        if (!hModule) {
            ErrorMessage("Could not load library %s with error %d", sModuleName, GetLastError());
            return nullptr;
        }
    }
    FARPROC result = GetProcAddress(hModule, sProcName);
    if (!result) {
        ErrorMessage("Could not resolve address for %s in module %s", sProcName, sModuleName);
    }
    return reinterpret_cast<void*>(result);
}

void* GetAddressByPattern(const char* sModuleName, const char* sPattern) {
    HMODULE hModule = GetModuleHandleA(sModuleName);
    if (!hModule) {
        hModule = LoadLibraryA(sModuleName);
        if (!hModule) {
            ErrorMessage("Could not load library %s with error %d", sModuleName, GetLastError());
            return nullptr;
        }
    }
    MODULEINFO mi;
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &mi, sizeof(mi))) {
        ErrorMessage("Could not get module information for : %s", sModuleName);
        return nullptr;
    }
    unsigned char* pModuleBase = static_cast<unsigned char*>(mi.lpBaseOfDll);
    size_t uModuleSize = mi.SizeOfImage;

    unsigned char abPattern[1024];
    unsigned char abMask[1024];
    size_t uPatternSize = ParsePattern(sPattern, abPattern, abMask);
    if (uPatternSize == 0) {
        ErrorMessage("Could not parse pattern : %s", sPattern);
        return nullptr;
    }

    void* pAddress = FindPattern(pModuleBase, uModuleSize, abPattern, abMask, uPatternSize);
    if (!pAddress) {
        ErrorMessage("Could not resolve address for pattern \"%s\" in module %s", sPattern, sModuleName);
    }
    return pAddress;
}

void PatchMemory(void* pAddress, void* pValue, size_t uSize) {
    DWORD flOldProtect;
    VirtualProtect(pAddress, uSize, PAGE_EXECUTE_READWRITE, &flOldProtect);
    memcpy(pAddress, pValue, uSize);
    VirtualProtect(pAddress, uSize, flOldProtect, &flOldProtect);
}