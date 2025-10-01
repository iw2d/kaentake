#include "pch.h"
#include "hook.h"
#include "wvs/wvsapp.h"
#include "wvs/stage.h"
#include "wvs/packet.h"
#include "wvs/util.h"
#include "ztl/ztl.h"

#include <windows.h>
#include <comdef.h>

#pragma comment(lib, "winmm.lib")
#pragma warning(disable : 4996)


class ZInetAddr : public sockaddr_in {
public:
    operator const struct sockaddr *() const {
        return (const struct sockaddr*)this;
    }
    operator const struct sockaddr_in *() const {
        return (const struct sockaddr_in*)this;
    }
};

class ZSocketBase {
private:
    SOCKET _m_hSocket;

public:
    operator SOCKET() {
        return _m_hSocket;
    }
    void CloseSocket() {
        if (_m_hSocket != INVALID_SOCKET) {
            closesocket(_m_hSocket);
            _m_hSocket = INVALID_SOCKET;
        }
    }
    void Socket(int type, int af, int protocol) {
        _m_hSocket = socket(af, type, protocol);
        if (_m_hSocket == INVALID_SOCKET) {
            throw WSAGetLastError();
        }
    }
};


ZRECYCLABLE(ZInetAddr, 0x00BF6A18)

class CClientSocket : public TSingleton<CClientSocket, 0x00BE7914> {
public:
    struct CONNECTCONTEXT {
        ZList<ZInetAddr> lAddr;
        ZInetAddr* posList;
        int bLogin;
    };
    static_assert(sizeof(CONNECTCONTEXT) == 0x1C);

    MEMBER_AT(HWND, 0x4, m_hWnd)
    MEMBER_AT(ZSocketBase, 0x8, m_sock)
    MEMBER_AT(CONNECTCONTEXT, 0xC, m_ctxConnect)
    MEMBER_HOOK(void, 0x00494CA3, Connect, const CONNECTCONTEXT& ctx)
};

void CClientSocket::Connect_hook(const CONNECTCONTEXT& ctx) {
    DEBUG_MESSAGE("CClientSocket::Connect");
    m_ctxConnect.lAddr.RemoveAll();
    m_ctxConnect.lAddr.AddTail(ctx.lAddr);
    m_ctxConnect.posList = ctx.posList;
    m_ctxConnect.bLogin = ctx.bLogin;
    m_ctxConnect.posList = m_ctxConnect.lAddr.GetHeadPosition();
    auto next = ZList<ZInetAddr>::GetNext(m_ctxConnect.posList);
    // CClientSocket::ClearSendReceiveCtx(this);
    reinterpret_cast<void(__thiscall*)(CClientSocket*)>(0x004969EE)(this);

    m_sock.CloseSocket();
    m_sock.Socket(SOCK_STREAM, AF_INET, 0);
    if (WSAAsyncSelect(m_sock, m_hWnd, 0x401, 0x33) == SOCKET_ERROR ||
            connect(m_sock, next, 16) != INVALID_SOCKET ||
            WSAGetLastError() != WSAEWOULDBLOCK) {
        // CClientSocket::OnConnect(this, 0);
        reinterpret_cast<void(__thiscall*)(CClientSocket*, int)>(0x00494ED1)(this, 0);
    }
}


struct ISMSG {
    unsigned int message;
    unsigned int wParam;
    int lParam;
};

class CInputSystem : public TSingleton<CInputSystem, 0x00BEC33C> {
};

class CConfig : public TSingleton<CConfig, 0x00BEBF9C> {
};

void CWvsApp::Constructor_hook() {
    DEBUG_MESSAGE("CWvsApp::CWvsApp");
    ms_pInstance = this;
    m_hWnd = nullptr;
    m_bPCOMInitialized = 0;
    m_hHook = nullptr;
    m_tUpdateTime = 0;
    m_bFirstUpdate = 1;
    construct(&m_sCmdLine);
    m_nGameStartMode = 0;
    m_bAutoConnect = 1;
    m_bShowAdBalloon = 0;
    m_bExitByTitleEscape = 0;
    m_hrZExceptionCode = S_OK;
    m_hrComErrorCode = S_OK;

    m_nGameStartMode = 2;
    m_dwMainThreadId = GetCurrentThreadId();

    OSVERSIONINFOA ovi;
    ovi.dwOSVersionInfoSize = 148;
    GetVersionExA(&ovi);
    m_bWin9x = ovi.dwPlatformId == 1;
    if (ovi.dwMajorVersion >= 6 && !m_nGameStartMode) {
        m_nGameStartMode = 2;
    }

    typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    auto fnIsWow64Process = reinterpret_cast<LPFN_ISWOW64PROCESS>(GetProcAddress(GetModuleHandleA("KERNEL32"), "IsWow64Process"));
    BOOL bIs64 = 0;
    if (fnIsWow64Process) {
        fnIsWow64Process(GetCurrentProcess(), &bIs64);
    }
    if (ovi.dwMajorVersion >= 6 && !bIs64) {
        // ResetLSP();
        reinterpret_cast<void(__cdecl*)()>(0x0044ED47)();
    }
}

void CWvsApp::SetUp_hook() {
    DEBUG_MESSAGE("CWvsApp::SetUp");
    // CWvsApp::InitializeAuth(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F7097)(this);
    srand(timeGetTime());
    // GetSEPrivilege();
    reinterpret_cast<void(__cdecl*)()>(0x0044E824)();

    // CWvsApp::InitializePCOM(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F6D77)(this);
    // CWvsApp::CreateMainWindow(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F6D97)(this);
    // TSingleton<CClientSocket::CreateInstance();
    reinterpret_cast<void(__cdecl*)()>(0x009F9E53)();
    // CWvsApp::ConnectLogin(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F6F27)(this);
    // TSingleton<CFuncKeyMappedMan>::CreateInstance();
    reinterpret_cast<void(__cdecl*)()>(0x009F9E98)();
    // TSingleton<CQuickslotKeyMappedMan>::CreateInstance();
    reinterpret_cast<void(__cdecl*)()>(0x009FA0CB)();
    // TSingleton<CMacroSysMan>::CreateInstance();
    reinterpret_cast<void(__cdecl*)()>(0x009F9EEE)();
    // CWvsApp::InitializeResMan(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F7159)(this);

    // CWvsApp::InitializeGr2D(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F7A3B)(this);
    // TSingleton<CInputSystem>::CreateInstance();
    auto pInputSystem = static_cast<CInputSystem*>(ZAllocEx<ZAllocAnonSelector>::s_Alloc(0x9D0));
    if (pInputSystem) {
        // CInputSystem::CInputSystem(pInputSystem);
        reinterpret_cast<void(__thiscall*)(CInputSystem*)>(0x009F821F)(pInputSystem);
    }
    // CInputSystem::Init(CInputSystem::GetInstance(), m_hWnd, m_ahInput);
    reinterpret_cast<void(__thiscall*)(CInputSystem*, HWND, void**)>(0x00599EBF)(pInputSystem, m_hWnd, m_ahInput);
    // CWvsApp::InitializeSound(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F82BC)(this);
    // CWvsApp::InitializeGameData(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F8B61)(this);
    // CWvsApp::CreateWndManager(this);
    reinterpret_cast<void(__thiscall*)(CWvsApp*)>(0x009F7034)(this);
    // CConfig::ApplySysOpt(TSingleton<CConfig>::ms_pInstance, nullptr, nullptr);
    reinterpret_cast<void(__thiscall*)(CConfig*, void*, void*)>(0x0049EA33)(CConfig::GetInstance(), nullptr, nullptr);

    // TSingleton<CActionMan>::CreateInstance()->Init();
    auto pActionMan = reinterpret_cast<void*(__cdecl*)()>(0x009F9DA6)();
    reinterpret_cast<void(__thiscall*)(void*)>(0x00406ABD)(pActionMan);
    // TSingleton<CAnimationDisplayer>::CreateInstance();
    reinterpret_cast<void*(__cdecl*)()>(0x009F9DFC)();
    // TSingleton<CMapleTVMan>::CreateInstance()->Init()
    auto pMapleTVMan = reinterpret_cast<void*(__cdecl*)()>(0x009F9F87)();
    reinterpret_cast<void(__thiscall*)(void*)>(0x00636F4E)(pMapleTVMan);
    // TSingleton<CQuestMan>::CreateInstance()->LoadDemand();
    auto pQuestMan = reinterpret_cast<void*(__cdecl*)()>(0x009F9AC2)();
    if (!reinterpret_cast<int(__thiscall*)(void*)>(0x0071D8DF)(pQuestMan)) {
        ErrorMessage("Failed to load quest data.");
    }
    // CQuestMan::LoadPartyQuestInfo(pQuestMan);
    reinterpret_cast<int(__thiscall*)(void*)>(0x00723341)(pQuestMan);
    // CQuestMan::LoadExclusive(pQuestMan);
    reinterpret_cast<int(__thiscall*)(void*)>(0x007247A1)(pQuestMan);
    // TSingleton<CMonsterBookMan>::CreateInstance()->LoadBook();
    auto pMonsterBookMan = reinterpret_cast<void*(__cdecl*)()>(0x009F9B73)();
    if (!reinterpret_cast<int(__thiscall*)(void*)>(0x0068487C)(pMonsterBookMan)) {
        ErrorMessage("Failed to load monster book data.");
    }
    // TSingleton<CRadioManager>::CreateInstance();
    reinterpret_cast<void*(__cdecl*)()>(0x009FA078)();

    // (CLogo*) operator new(0x38); -> (CLogin*) operator new(0x28C);
    CStage* pStage = static_cast<CStage*>(ZAllocEx<ZAllocAnonSelector>::s_Alloc(0x28C));
    if (pStage) {
        // CLogo::CLogo(pStage); -> CLogin::Clogin(pStage);
        reinterpret_cast<void(__thiscall*)(void*)>(0x005F3C59)(pStage);
    }
    // set_stage(pStage, nullptr);
    reinterpret_cast<void(__cdecl*)(CStage*, void*)>(0x00777347)(pStage, nullptr);
}

void CWvsApp::CallUpdate_hook(int tCurTime) {
    if (m_bFirstUpdate) {
        m_tUpdateTime = tCurTime;
        m_bFirstUpdate = 0;
    }
    while (tCurTime - m_tUpdateTime > 0) {
        auto pStage = get_stage();
        if (pStage) {
            pStage->Update();
        }
        // CWndMan::s_Update();
        reinterpret_cast<void(__cdecl*)()>(0x009E47C3)();
        m_tUpdateTime += 30;
        if (tCurTime - m_tUpdateTime > 0) {
            get_gr()->UpdateCurrentTime(m_tUpdateTime);
        }
    }
    get_gr()->UpdateCurrentTime(tCurTime);
}

void CWvsApp::Run_hook(int* pbTerminate) {
    MSG msg;
    ISMSG isMsg;
    memset(&msg, 0, sizeof(msg));
    memset(&isMsg, 0, sizeof(isMsg));
    if (CClientSocket::IsInstantiated()) {
        // CClientSocket::ManipulatePacket(TSingleton<CClientSocket>::GetInstance());
        reinterpret_cast<void(__thiscall*)(CClientSocket*)>(0x0049651D)(CClientSocket::GetInstance());
    }
    do {
        DWORD dwRet = MsgWaitForMultipleObjects(3, m_ahInput, FALSE, 0, 0xFF);
        if (dwRet <= 2) {
            // CInputSystem::UpdateDevice(TSingleton<CInputSystem>::GetInstance(), dwRet);
            reinterpret_cast<void(__thiscall*)(CInputSystem*, int)>(0x0059A2E9)(CInputSystem::GetInstance(), dwRet);
            do {
                // if (!CInputSystem::GetISMessage(TSingleton<CInputSystem>::GetInstance(), &isMsg))
                if (!reinterpret_cast<int(__thiscall*)(CInputSystem*, ISMSG*)>(0x0059A306)(CInputSystem::GetInstance(), &isMsg)) {
                    break;
                }
                // CWvsApp::ISMsgProc(this, isMsg.message, isMsg.wParam, isMsg.lParam);
                reinterpret_cast<void(__thiscall*)(CWvsApp*, unsigned int, unsigned int, int)>(0x009F97B7)(this, isMsg.message, isMsg.wParam, isMsg.lParam);
            } while (!*pbTerminate);
        } else if (dwRet == 3) {
            do {
                if (!PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
                // CWvsApp::ExtractComErrorCode(this, &hr);
                if (m_hrComErrorCode) {
                    HRESULT hr = m_hrComErrorCode;
                    m_hrComErrorCode = S_OK;
                    m_hrZExceptionCode = S_OK;
                    _com_raise_error(hr, nullptr);
                }
                // CWvsApp::ExtractZExceptionCode(this, &hr)
                if (m_hrZExceptionCode) {
                    HRESULT hr = m_hrZExceptionCode;
                    m_hrComErrorCode = S_OK;
                    m_hrZExceptionCode = S_OK;
                    struct {
                        HRESULT m_hr;
                    } exception = {hr};
                    throw exception;
                }
            } while (!*pbTerminate && msg.message != WM_QUIT);
        } else {
            // CInputSystem::GenerateAutoKeyDown(TSingleton<CInputSystem>::GetInstance(), &isMsg);
            if (reinterpret_cast<int(__thiscall*)(CInputSystem*, ISMSG*)>(0x0059B2D2)(CInputSystem::GetInstance(), &isMsg)) {
                // CWvsApp::ISMsgProc(this, isMsg.message, isMsg.wParam, isMsg.lParam);
                reinterpret_cast<void(__thiscall*)(CWvsApp*, unsigned int, unsigned int, int)>(0x009F97B7)(this, isMsg.message, isMsg.wParam, isMsg.lParam);
            }
            int tCurTime = get_gr()->nextRenderTime;
            CallUpdate_hook(tCurTime);
            // CWndMan::RedrawInvalidatedWindows();
            reinterpret_cast<void(__cdecl*)()>(0x009E4547)();
            get_gr()->RenderFrame();
            Sleep(1);
        }
    } while (!*pbTerminate && msg.message != WM_QUIT);

    if (msg.message == WM_QUIT) {
        PostQuitMessage(0);
    }
}


class CSystemInfo {
public:
    unsigned char SupportId[16];
    unsigned char MachineId[16];

    virtual ~CSystemInfo() = default;
};

class CLogin {
public:
    struct WORLDITEM;
    struct BALLOON;

    MEMBER_AT(int, 0x170, m_bRequestSent)
    MEMBER_AT(ZArray<WORLDITEM>, 0x18C, m_aWorldItem)
    MEMBER_AT(ZArray<BALLOON>, 0x204, m_aBalloon)
    MEMBER_HOOK(int, 0x005F6952, SendCheckPasswordPacket, char* sID, char* sPasswd)
};

int CLogin::SendCheckPasswordPacket_hook(char* sID, char* sPasswd) {
    if (m_bRequestSent) {
        return 0;
    }
    // m_aWorldItem.RemoveAll();
    reinterpret_cast<void(__thiscall*)(ZArray<WORLDITEM>*)>(0x005FDDE3)(&m_aWorldItem);
    // m_aBalloon.RemoveAll();
    reinterpret_cast<void(__thiscall*)(ZArray<BALLOON>*)>(0x005FDF26)(&m_aBalloon);
    CSystemInfo si;
    // CSystemInfo::Init(&si);
    reinterpret_cast<void(__thiscall*)(CSystemInfo*)>(0x00A54BD0)(&si);

    COutPacket oPacket(1); // CP_CheckPassword
    oPacket.EncodeStr(sID);
    oPacket.EncodeStr(sPasswd);
    oPacket.EncodeBuffer(si.MachineId, 16);
    oPacket.Encode4(0); // CSystemInfo::GetGameRoomClient(&v15)
    oPacket.Encode1(CWvsApp::GetInstance()->m_nGameStartMode);
    oPacket.Encode1(0);
    oPacket.Encode1(0);
    oPacket.Encode4(0); // CConfig::GetPartnerCode(TSingleton<CConfig>::ms_pInstance._m_pStr)
    // CClientSocket::SendPacket(CClientSocket::GetInstance(), oPacket);
    reinterpret_cast<void(__thiscall*)(CClientSocket*, const COutPacket&)>(0x0049637B)(CClientSocket::GetInstance(), oPacket);
    return 1;
}


void AttachClientBypass() {
    ATTACH_HOOK(CClientSocket::Connect, CClientSocket::Connect_hook);
    ATTACH_HOOK(CWvsApp::Constructor, CWvsApp::Constructor_hook);
    ATTACH_HOOK(CWvsApp::SetUp, CWvsApp::SetUp_hook);
    ATTACH_HOOK(CWvsApp::Run, CWvsApp::Run_hook);
    ATTACH_HOOK(CLogin::SendCheckPasswordPacket, CLogin::SendCheckPasswordPacket_hook);

    PatchRetZero(0x009FEC62); // CWvsApp::EnableWinKey
    PatchRetZero(0x009F18C9); // ShowStartUpWndModal
    PatchRetZero(0x00422C7E); // ShowAdBalloon
    PatchRetZero(0x009F191B); // SendHSLog
}