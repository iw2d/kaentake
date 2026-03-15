#include "pch.h"
#include "hook.h"
#include "constants.h"
#include "ztl/ztl.h"
#include <windows.h>

ZALLOC_GLOBAL
ZALLOCEX(ZAllocAnonSelector, 0x00BF0B00)
ZALLOCEX(ZAllocStrSelector<char>, 0x00BF0A90)
ZALLOCEX(ZAllocStrSelector<wchar_t>, 0x00BF0BA8)

extern "C" __declspec(dllexport) VOID DummyExport() {}

char* g_sServerHost = nullptr;
long g_nServerPort = 0;

void ProcessCommandLine() {
    if (!CONSTANTS_CMD_HOST) {
        return;
    }
    char sBuffer[1024];
    long nPort;
    if (sscanf_s(GetCommandLineA(), "%s %d", sBuffer, sizeof(sBuffer), &nPort) == 2) {
        g_sServerHost = _strdup(sBuffer);
        g_nServerPort = nPort;
    }
}

void ProcessConfigFile() {
    if (!CONSTANTS_INI_HOST || g_sServerHost) {
        return;
    }
    char sBuffer[1024];
    if (GetPrivateProfileStringA("config", "host", "", sBuffer, sizeof(sBuffer), ".\\" CONSTANTS_INI_NAME) == 0) {
        return;
    }
    g_sServerHost = _strdup(sBuffer);
    if (GetPrivateProfileStringA("config", "port", "", sBuffer, sizeof(sBuffer), ".\\" CONSTANTS_INI_NAME) == 0) {
        return;
    }
    g_nServerPort = atoi(sBuffer);
}


BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        ProcessCommandLine();
        ProcessConfigFile();
        AttachSystemHooks();
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}