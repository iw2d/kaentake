#include "pch.h"
#include "hook.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>
#include <ws2spi.h>
#include <ws2tcpip.h>
#include <intrin.h>

#pragma comment(lib, "ws2_32.lib")


typedef decltype(&SetUnhandledExceptionFilter) SetUnhandledExceptionFilter_t;
static SetUnhandledExceptionFilter_t SetUnhandledExceptionFilter_orig = reinterpret_cast<SetUnhandledExceptionFilter_t>(GetAddress("KERNEL32", "SetUnhandledExceptionFilter"));

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter_hook(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
    // ZExceptionHandler::ZExceptionHandler - after dynamic initializers for ZAllocEx<T>::_s_alloc
    if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == 0x00796FDD) {
        AttachClientHooks();
    }
    return SetUnhandledExceptionFilter_orig(lpTopLevelExceptionFilter);
}


typedef decltype(&CreateMutexA) CreateMutexA_t;
static CreateMutexA_t CreateMutexA_orig = reinterpret_cast<CreateMutexA_t>(GetAddress("KERNEL32", "CreateMutexA"));

HANDLE WINAPI CreateMutexA_hook(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) {
    DEBUG_MESSAGE("CreateMutexA : %s", lpName);
    if (lpName && !strcmp(lpName, "WvsClientMtx")) {
        char sMutex[1024];
        sprintf_s(sMutex, 1024, "%s-%d", lpName, GetCurrentProcessId());
        lpName = sMutex;
        return CreateMutexA_orig(lpMutexAttributes, bInitialOwner, sMutex);
    }
    return CreateMutexA_orig(lpMutexAttributes, bInitialOwner, lpName);
}


typedef decltype(&CreateWindowExA) CreateWindowExA_t;
static CreateWindowExA_t CreateWindowExA_orig = reinterpret_cast<CreateWindowExA_t>(GetAddress("USER32", "CreateWindowExA"));
static WNDPROC g_WndProc;

LRESULT WndProc_hook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_SETCURSOR) {
        if (LOWORD(lParam) != HTCLIENT) {
            while (ShowCursor(TRUE) < 0)
                ;
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return 0;
        }
    }
    return CallWindowProcA(g_WndProc, hWnd, msg, wParam, lParam);
}

HWND WINAPI CreateWindowExA_hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    if (!lpClassName || strcmp(lpClassName, "MapleStoryClass") != 0) {
        return CreateWindowExA_orig(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    }
    HWND hWnd = CreateWindowExA_orig(dwExStyle, lpClassName, lpWindowName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    g_WndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WndProc_hook)));
    return hWnd;
}


typedef decltype(&RegCreateKeyExA) RegCreateKeyExA_t;
static RegCreateKeyExA_t RegCreateKeyExA_orig = reinterpret_cast<RegCreateKeyExA_t>(GetAddress("ADVAPI32", "RegCreateKeyExA"));

LSTATUS WINAPI RegCreateKeyExA_hook(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition) {
    return RegCreateKeyExA_orig(HKEY_CURRENT_USER, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}


typedef decltype(&WSPStartup) WSPStartup_t;
static WSPStartup_t WSPStartup_orig = reinterpret_cast<WSPStartup_t>(GetAddress("MSWSOCK", "WSPStartup"));
static WSPPROC_TABLE g_ProcTable;
static ULONG g_uNexonAddress;

constexpr const char* g_asOriginalAddress[] = {
    "63.251.217.2",
    "63.251.217.3",
    "63.251.217.4",
};

int WSPAPI WSPConnect_hook(SOCKET s, const struct sockaddr FAR* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno) {
    char sName[INET_ADDRSTRLEN];
    InetNtopA(AF_INET, &((sockaddr_in*)name)->sin_addr, sName, INET_ADDRSTRLEN);
    for (auto sAddress : g_asOriginalAddress) {
        if (strcmp(sName, sAddress)) {
            continue;
        }
        g_uNexonAddress = ((sockaddr_in*)name)->sin_addr.S_un.S_addr;
        InetPtonA(AF_INET, "127.0.0.1", &((sockaddr_in*)name)->sin_addr.S_un.S_addr);
        break;
    }
    return g_ProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
}

int WSPAPI WSPGetPeerName_hook(SOCKET s, struct sockaddr* name, LPINT namelen, LPINT lpErrNo) {
    int result = g_ProcTable.lpWSPGetPeerName(s, name, namelen, lpErrNo);
    ((sockaddr_in*)name)->sin_addr.S_un.S_addr = g_uNexonAddress;
    return result;
}

int WSPAPI WSPStartup_hook(WORD wVersionRequested, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFOW lpProtocolInfo, WSPUPCALLTABLE UpcallTable, LPWSPPROC_TABLE lpProcTable) {
    int result = WSPStartup_orig(wVersionRequested, lpWSPData, lpProtocolInfo, UpcallTable, lpProcTable);
    g_ProcTable = *lpProcTable;
    lpProcTable->lpWSPConnect = &WSPConnect_hook;
    lpProcTable->lpWSPGetPeerName = &WSPGetPeerName_hook;
    return result;
}


void AttachSystemHooks() {
    ATTACH_HOOK(SetUnhandledExceptionFilter_orig, SetUnhandledExceptionFilter_hook);
    ATTACH_HOOK(CreateMutexA_orig, CreateMutexA_hook);
    ATTACH_HOOK(CreateWindowExA_orig, CreateWindowExA_hook);
    ATTACH_HOOK(RegCreateKeyExA_orig, RegCreateKeyExA_hook);
    ATTACH_HOOK(WSPStartup_orig, WSPStartup_hook);
}