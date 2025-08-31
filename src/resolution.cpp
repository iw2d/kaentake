#include "hook.h"
#include "wvs/config.h"
#include "wvs/ctrlwnd.h"
#include "wvs/wndman.h"
#include "wvs/stage.h"
#include "wvs/rtti.h"
#include "wvs/util.h"
#include "ztl/zalloc.h"
#include "ztl/zcom.h"
#include "ztl/ztl.h"
#include <windows.h>


static ZRef<CCtrlComboBox> g_cbResolution;
static int g_nResolution = 0;
static int g_nScreenWidth = 800;
static int g_nScreenHeight = 600;
static int g_nAdjustCenterY = 0;

int get_screen_width() {
    return g_nScreenWidth;
}

int get_screen_height() {
    return g_nScreenHeight;
}

int get_adjust_cy() {
    return g_nAdjustCenterY;
}

void set_screen_resolution(int nResolution, bool bSave) {
    int nScreenWidth = 800;
    int nScreenHeight = 600;
    switch (nResolution) {
    case 1:
        nScreenWidth = 1024;
        nScreenHeight = 768;
        break;
    case 2:
        nScreenWidth = 1366;
        nScreenHeight = 768;
        break;
    case 3:
        nScreenWidth = 1600;
        nScreenHeight = 900;
        break;
    case 4:
        nScreenWidth = 1920;
        nScreenHeight = 1080;
        break;
    }
    if (nScreenWidth != g_nScreenWidth || nScreenHeight != g_nScreenHeight) {
        IWzGr2D_DX9Ptr gr = get_gr()->GetInner();
        if (SUCCEEDED(gr->screenResolution(nScreenWidth, nScreenHeight))) {
            g_nScreenWidth = nScreenWidth;
            g_nScreenHeight = nScreenHeight;
            g_nAdjustCenterY = (g_nScreenHeight - 600) / 2;
            gr->AdjustCenter(0, -g_nAdjustCenterY);
            // CWndMan::ResetOrgWindow
            CWndMan::GetInstance()->m_pOrgWindow->RelMove(get_screen_width() / -2, get_screen_height() / -2 - get_adjust_cy());
        }
    }
    if (bSave) {
        g_nResolution = nResolution;
    }
}


static auto set_stage = reinterpret_cast<void(__cdecl*)(CStage*, void*)>(0x00777347);
void __cdecl set_stage_hook(CStage* pStage, void* pParam) {
    set_stage(pStage, pParam);
    if (pStage) {
        // CField::ms_RTTI_CField
        if (pStage->IsKindOf(reinterpret_cast<const CRTTI*>(0x00BED758))) {
            set_screen_resolution(g_nResolution, 0);
        } else {
            set_screen_resolution(0, 0);
        }
    }
}


static auto CConfig__LoadGlobal = reinterpret_cast<void(__thiscall*)(CConfig*)>(0x0049C441);
void __fastcall CConfig__LoadGlobal_hook(CConfig* pThis, void* _EDX) {
    CConfig__LoadGlobal(pThis);
    g_nResolution = pThis->GetOpt_Int(CConfig::GLOBAL_OPT, "soScreenResolution", 0, 0, 4);
}

static auto CConfig__SaveGlobal = reinterpret_cast<void(__thiscall*)(CConfig*)>(0x0049C8E7);
void __fastcall CConfig__SaveGlobal_hook(CConfig* pThis, void* _EDX) {
    CConfig__SaveGlobal(pThis);
    pThis->SetOpt_Int(CConfig::GLOBAL_OPT, "soScreenResolution", g_nResolution);
}

static auto CConfig__ApplySysOpt = reinterpret_cast<void(__thiscall*)(CConfig*, void*, int)>(0x0049EA33);
void __fastcall CConfig__ApplySysOpt_hook(CConfig* pThis, void* _EDX, void* pSysOpt, int bApplyVideo) {
    CConfig__ApplySysOpt(pThis, pSysOpt, bApplyVideo);
    if (pSysOpt && bApplyVideo && g_cbResolution) {
        set_screen_resolution(g_cbResolution->m_nSelect, true);
    }
}


class CUISysOpt;

static auto CUISysOpt__OnCreate = reinterpret_cast<void(__thiscall*)(CUISysOpt*, void*)>(0x00994163);
void __fastcall CUISysOpt__OnCreate_hook(CUISysOpt* pThis, void* _EDX, void* pData) {
    CUISysOpt__OnCreate(pThis, pData);

    CCtrlComboBox::CREATEPARAM paramComboBox;
    paramComboBox.nBackColor = 0xFFEEEEEE;
    paramComboBox.nBackFocusedColor = 0xFFA5A198;
    paramComboBox.nBorderColor = 0xFF999999;

    g_cbResolution = new CCtrlComboBox();
    g_cbResolution->CreateCtrl(pThis, 2000, 0, 67, 278, 175, 18, &paramComboBox);
    const char* asResolution[] = {
            "800 x 600",
            "1024 x 768",
            "1366 x 768",
            "1600 x 900",
            "1920 x 1080",
    };
    unsigned int dwResolutionParam = 0;
    for (auto sResolution : asResolution) {
        g_cbResolution->AddItem(sResolution, dwResolutionParam++);
    }
    g_cbResolution->SetSelect(g_nResolution);
}

static auto CUISysOpt__dtor = reinterpret_cast<void(__thiscall*)(CUISysOpt*)>(0x007FF4AA);
void __fastcall CUISysOpt__dtor_hook(CUISysOpt* pThis) {
    CUISysOpt__dtor(pThis);
    g_cbResolution = nullptr;
}


class CInputSystem {
public:
    MEMBER_AT(HWND, 0x0, m_hWnd)
    MEMBER_AT(IWzVector2DPtr, 0x9B0, m_pVectorCursor)
};

static auto CInputSystem__SetCursorVectorPos = reinterpret_cast<void(__thiscall*)(CInputSystem*, int, int)>(0x0059A0CB);
void __fastcall CInputSystem__SetCursorVectorPos_hook(CInputSystem* pThis, void* _EDX, int x, int y) {
    pThis->m_pVectorCursor->RelMove(x - get_screen_width() / 2, y - get_screen_height() / 2 - get_adjust_cy());
}

static auto CInputSystem__SetCursorPos = reinterpret_cast<int(__thiscall*)(CInputSystem*, int, int)>(0x0059A887);
int __fastcall CInputSystem__SetCursorPos_hook(CInputSystem* pThis, void* _EDX, int x, int y) {
    POINT pt;
    pt.x = zclamp(x, 0, get_screen_width());
    pt.y = zclamp(y, 0, get_screen_height());
    CInputSystem__SetCursorVectorPos_hook(pThis, _EDX, pt.x, pt.y);
    return ClientToScreen(pThis->m_hWnd, &pt) && SetCursorPos(pt.x, pt.y);
}


void AttachResolutionMod() {
    ATTACH_HOOK(set_stage, set_stage_hook);
    ATTACH_HOOK(CConfig__LoadGlobal, CConfig__LoadGlobal_hook);
    ATTACH_HOOK(CConfig__SaveGlobal, CConfig__SaveGlobal_hook);
    ATTACH_HOOK(CConfig__ApplySysOpt, CConfig__ApplySysOpt_hook);
    ATTACH_HOOK(CUISysOpt__OnCreate, CUISysOpt__OnCreate_hook);
    ATTACH_HOOK(CUISysOpt__dtor, CUISysOpt__dtor_hook);
    // CUISysOpt::OnCreate - hide monster info combo box
    Patch4(0x00994525 + 1, 0);

    ATTACH_HOOK(CInputSystem__SetCursorVectorPos, CInputSystem__SetCursorVectorPos_hook);
    ATTACH_HOOK(CInputSystem__SetCursorPos, CInputSystem__SetCursorPos_hook);
    // CWvsApp::CreateWndManager - maximum bounds for CWndMan
    Patch4(0x009F707D + 1, 1920);
    Patch4(0x009F7078 + 1, 1080);
}