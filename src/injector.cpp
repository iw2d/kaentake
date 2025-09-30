#include "pch.h"
#include "hook.h"
#include "ztl/ztl.h"
#include <windows.h>

ZALLOC_GLOBAL
ZALLOCEX(ZAllocAnonSelector, 0x00BF0B00)
ZALLOCEX(ZAllocStrSelector<char>, 0x00BF0A90)
ZALLOCEX(ZAllocStrSelector<wchar_t>, 0x00BF0BA8)

extern "C" __declspec(dllexport) VOID DummyExport() {}


BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        AttachSystemHooks();
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}