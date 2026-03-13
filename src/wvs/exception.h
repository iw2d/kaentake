#pragma once
#include "ztl/ztl.h"


enum ExceptionCode {
    EC_PATCH = 0x20000000,
    EC_DISCONNECT_BEGIN = 0x21000000,
    EC_CONNECT_TO_GAME_FAILED = 0x21000001,
    EC_CONNECTION_FROM_GAME_CLOSED = 0x21000002,
    EC_FAILED_PROTOCOL_WITH_GAME = 0x21000003,
    EC_FORCE_DISCONNECT = 0x21000004,
    EC_DISCONNECT_BY_MALICIOUS_PROCESS = 0x21000005,
    EC_DISCONNECT_END = 0x21000006,
    EC_TERMINATE_BEGIN = 0x22000000,
    EC_CONNECT_TO_LOGIN_FAILED = 0x22000001,
    EC_CONNECTION_FROM_LOGIN_CLOSED = 0x22000002,
    EC_NOT_ENOUGH_MEMORY = 0x22000003,
    EC_NO_DATA_PACKAGE = 0x22000004,
    EC_INVALID_GAME_DATA_VERSION = 0x22000005,
    EC_INVALID_GAME_DATA = 0x22000006,
    EC_INVALID_CLIENT_VERSION = 0x22000007,
    EC_FAILED_CRITICAL_PROTOCOL_WITH_GAME = 0x22000008,
    EC_WEB_LOGIN_NEEDED = 0x22000009,
    EC_CLIENTCRC_FAILED = 0x2200000A,
    EC_DOWNLOAD_FULL_CLIENT = 0x2200000B,
    EC_AUTH_SETLOCALE_FAILED = 0x2200000C,
    EC_AUTH_COINIT_FAILED = 0x2200000D,
    EC_TERMINATE_END = 0x2200000E,
};

class CMSException : public ZException {
public:
    CMSException(HRESULT hr) : ZException(hr) {
    }
};

class CPatchException : public CMSException {
public:
    struct PATCHINFO {
        unsigned short nCurrentVersion;
        unsigned short nTargetVersion;
        unsigned char sReserved[768];
        char sCommandLine[256];
        char sRootPath[256];
    };
    static_assert(sizeof(PATCHINFO) == 0x504);

    PATCHINFO m_pi;

    CPatchException(unsigned short nTargetVersion) : CMSException(ExceptionCode::EC_PATCH) {
        reinterpret_cast<void(__thiscall*)(CPatchException*, unsigned short)>(0x0051E834)(this, nTargetVersion);
    }
};

class CDisconnectException : public CMSException {
public:
    CDisconnectException(HRESULT hr) : CMSException(hr) {
    }
};

class CTerminateException : public CMSException {
public:
    CTerminateException(HRESULT hr) : CMSException(hr) {
    }
};