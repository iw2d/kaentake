#include "debug.h"
#include <windows.h>
#include <detours.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(STARTUPINFOA);

    const char* sApplicationName = lpCmdLine && *lpCmdLine ? lpCmdLine : "MapleStory.exe";
    if (!DetourCreateProcessWithDllExA(sApplicationName, lpCmdLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi, "kaentake.dll", NULL)) {
        DWORD dwError = GetLastError();
        LPSTR sErrorMessage = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dwError, 0, (LPSTR)&sErrorMessage, 0, nullptr);
        ErrorMessage("Could not start %s [%d]\n%s", sApplicationName, dwError, sErrorMessage);
        LocalFree(sErrorMessage);
        return 1;
    }
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD dwExitCode;
    if (!GetExitCodeProcess(pi.hProcess, &dwExitCode)) {
        ErrorMessage("GetExitCodeProcess failed [%d]", GetLastError());
        return 1;
    }
    return 0;
}