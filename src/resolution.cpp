#include "hook.h"
#include "wvs/wnd.h"
#include "wvs/ctrlwnd.h"
#include "wvs/stage.h"
#include "wvs/rtti.h"
#include "wvs/util.h"
#include "ztl/zalloc.h"
#include "ztl/zcom.h"
#include "ztl/ztl.h"
#include "ztl/tsingleton.h"
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

void set_screen_resolution(int nResolution, bool bSave);


static auto set_stage = reinterpret_cast<void(__cdecl*)(CStage*, void*)>(0x00777347);
void __cdecl set_stage_hook(CStage* pStage, void* pParam) {
    set_stage(pStage, pParam);
    if (!pStage) {
        return;
    }
    // CInterStage::ms_RTTI_CInterStage
    if (pStage->IsKindOf(reinterpret_cast<const CRTTI*>(0x00BED874))) {
        return;
    }
    // CField::ms_RTTI_CField
    if (pStage->IsKindOf(reinterpret_cast<const CRTTI*>(0x00BED758))) {
        set_screen_resolution(g_nResolution, 0);
    } else {
        set_screen_resolution(0, 0);
    }
}


class CConfig : public TSingleton<CConfig, 0x00BEBF9C> {
public:
    enum {
        GLOBAL_OPT = 0x0,
        LAST_CONNECT_INFO = 0x1,
        CHARACTER_OPT = 0x2,
    };
    MEMBER_HOOK(void, 0x0049C441, LoadGlobal)
    MEMBER_HOOK(void, 0x0049C8E7, SaveGlobal)
    MEMBER_HOOK(void, 0x0049EA33, ApplySysOpt, void* pSysOpt, int bApplyVideo)

    int GetOpt_Int(int nType, const char* sKey, int nDefaultValue, int nLowBound, int nHighBound) {
        return reinterpret_cast<int(__thiscall*)(CConfig*, int, const char*, int, int, int)>(0x0049EF65)(this, nType, sKey, nDefaultValue, nLowBound, nHighBound);
    }
    void SetOpt_Int(int nType, const char* sKey, int nValue) {
        reinterpret_cast<void(__thiscall*)(CConfig*, int, const char*, int)>(0x0049EFB5)(this, nType, sKey, nValue);
    }
};

void CConfig::LoadGlobal_hook() {
    CConfig::LoadGlobal_orig(this);
    g_nResolution = GetOpt_Int(CConfig::GLOBAL_OPT, "soScreenResolution", 0, 0, 4);
}

void CConfig::SaveGlobal_hook() {
    CConfig::SaveGlobal_orig(this);
    SetOpt_Int(CConfig::GLOBAL_OPT, "soScreenResolution", g_nResolution);
}

void CConfig::ApplySysOpt_hook(void* pSysOpt, int bApplyVideo) {
    CConfig::ApplySysOpt_orig(this, pSysOpt, bApplyVideo);
    if (pSysOpt && bApplyVideo && g_cbResolution) {
        set_screen_resolution(g_cbResolution->m_nSelect, true);
    }
}


class CUISysOpt {
public:
    MEMBER_HOOK(void, 0x00994163, OnCreate, void* pData)
    MEMBER_HOOK(void, 0x007FF4AA, Destructor)
};

void CUISysOpt::OnCreate_hook(void* pData) {
    CUISysOpt::OnCreate_orig(this, pData);

    CCtrlComboBox::CREATEPARAM paramComboBox;
    paramComboBox.nBackColor = 0xFFEEEEEE;
    paramComboBox.nBackFocusedColor = 0xFFA5A198;
    paramComboBox.nBorderColor = 0xFF999999;

    g_cbResolution = new CCtrlComboBox();
    g_cbResolution->CreateCtrl(this, 2000, 0, 67, 278, 175, 18, &paramComboBox);
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

void CUISysOpt::Destructor_hook() {
    CUISysOpt::Destructor_orig(this);
    g_cbResolution = nullptr;
}


class CInputSystem {
public:
    MEMBER_AT(HWND, 0x0, m_hWnd)
    MEMBER_AT(IWzVector2DPtr, 0x9B0, m_pVectorCursor)
    MEMBER_HOOK(void, 0x0059A0CB, SetCursorVectorPos, int x, int y)
    MEMBER_HOOK(int, 0x0059A887, SetCursorPos, int x, int y)
};

void CInputSystem::SetCursorVectorPos_hook(int x, int y) {
    m_pVectorCursor->RelMove(x - get_screen_width() / 2, y - get_screen_height() / 2 - get_adjust_cy());
}

int CInputSystem::SetCursorPos_hook(int x, int y) {
    POINT pt;
    pt.x = zclamp(x, 0, get_screen_width());
    pt.y = zclamp(y, 0, get_screen_height());
    SetCursorVectorPos_hook(pt.x, pt.y);
    return ClientToScreen(m_hWnd, &pt) && SetCursorPos(pt.x, pt.y);
}


class CWvsPhysicalSpace2D : public TSingleton<CWvsPhysicalSpace2D, 0x00BEBFA0> {
public:
    MEMBER_AT(RECT, 0x24, m_rcMBR)
};

class CMapLoadable {
public:
    MEMBER_AT(IWzPropertyPtr, 0x2C, m_pPropFieldInfo)
    MEMBER_AT(RECT, 0xF0, m_rcViewRange)
    MEMBER_HOOK(void, 0x00641EF1, RestoreViewRange)
};

void CMapLoadable::RestoreViewRange_hook() {
    auto pSpace2D = CWvsPhysicalSpace2D::GetInstance();
    m_rcViewRange.left = get_int32(m_pPropFieldInfo->item[L"VRLeft"], pSpace2D->m_rcMBR.left - 20) + get_screen_width() / 2;
    m_rcViewRange.top = get_int32(m_pPropFieldInfo->item[L"VRTop"], pSpace2D->m_rcMBR.top - 60) + get_screen_height() / 2;
    m_rcViewRange.right = get_int32(m_pPropFieldInfo->item[L"VRRight"], pSpace2D->m_rcMBR.right + 20) - get_screen_width() / 2;
    m_rcViewRange.bottom = get_int32(m_pPropFieldInfo->item[L"VRBottom"], pSpace2D->m_rcMBR.bottom + 190) - get_screen_height() / 2;
    if (m_rcViewRange.right - m_rcViewRange.left <= 0) {
        m_rcViewRange.left = (m_rcViewRange.left + m_rcViewRange.right) / 2;
        m_rcViewRange.right = (m_rcViewRange.left + m_rcViewRange.right) / 2;
    }
    if (m_rcViewRange.bottom - m_rcViewRange.top <= 0) {
        m_rcViewRange.top = (m_rcViewRange.top + m_rcViewRange.bottom) / 2;
        m_rcViewRange.bottom = (m_rcViewRange.top + m_rcViewRange.bottom) / 2;
    }
    m_rcViewRange.top += get_adjust_cy();
    m_rcViewRange.bottom += get_adjust_cy();
}

static auto CMapLoadable__MakeGrid_jmp = 0x0063EAD6;
static auto CMapLoadable__MakeGrid_ret = 0x0063EADC;
void __declspec(naked) CMapLoadable__MakeGrid_hook() {
    __asm {
        sar     ecx, 1                          ; overwritten instructions
        neg     eax
        sub     eax, ecx
        sub     eax, g_nAdjustCenterY           ; eax -= g_nAdjustCenterY
        jmp     [ CMapLoadable__MakeGrid_ret ]
    }
}


class CWndMan : public CWnd, public TSingleton<CWndMan, 0x00BEC20C> {
public:
    MEMBER_AT(IWzVector2DPtr, 0xDC, m_pOrgWindow)
};

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
            // CMapLoadable::OnEventChangeScreenResolution
            CMapLoadable* field = reinterpret_cast<CMapLoadable*(__cdecl*)()>(0x00437A0C)();
            if (field) {
                // CMapLoadable::RestoreViewRange
                field->RestoreViewRange_hook();
                // CMapLoadable::ReloadBack
                reinterpret_cast<void(__thiscall*)(CMapLoadable*)>(0x00644491)(field);
            }
        }
    }
    if (bSave) {
        g_nResolution = nResolution;
    }
}


static auto CUIToolTip__MakeLayer_jmp1 = 0x008F32CC;
static auto CUIToolTip__MakeLayer_ret1 = 0x008F32D1;
void __declspec(naked) CUIToolTip__MakeLayer_hook1() {
    __asm {
        mov     eax, g_nScreenWidth
        sub     eax, 1
        jmp     [ CUIToolTip__MakeLayer_ret1 ]
    };
}

static auto CUIToolTip__MakeLayer_jmp2 = 0x008F32DF;
static auto CUIToolTip__MakeLayer_ret2 = 0x008F32E4;
void __declspec(naked) CUIToolTip__MakeLayer_hook2() {
    __asm {
        mov     eax, g_nScreenHeight
        sub     eax, 1
        jmp     [ CUIToolTip__MakeLayer_ret2 ]
    };
}


void AttachResolutionMod() {
    ATTACH_HOOK(set_stage, set_stage_hook);
    ATTACH_HOOK(CConfig::LoadGlobal_orig, CConfig::LoadGlobal_hook);
    ATTACH_HOOK(CConfig::SaveGlobal_orig, CConfig::SaveGlobal_hook);
    ATTACH_HOOK(CConfig::ApplySysOpt_orig, CConfig::ApplySysOpt_hook);
    ATTACH_HOOK(CUISysOpt::OnCreate_orig, CUISysOpt::OnCreate_hook);
    ATTACH_HOOK(CUISysOpt::Destructor_orig, CUISysOpt::Destructor_hook);
    // CUISysOpt::OnCreate - hide monster info combo box
    Patch4(0x00994525 + 1, 0);

    ATTACH_HOOK(CInputSystem::SetCursorVectorPos_orig, CInputSystem::SetCursorVectorPos_hook);
    ATTACH_HOOK(CInputSystem::SetCursorPos_orig, CInputSystem::SetCursorPos_hook);
    ATTACH_HOOK(CMapLoadable::RestoreViewRange_orig, CMapLoadable::RestoreViewRange_hook);
    PatchJmp(CMapLoadable__MakeGrid_jmp, &CMapLoadable__MakeGrid_hook);
    // CWvsApp::CreateWndManager - maximum bounds for CWndMan
    Patch4(0x009F707D + 1, 1920);
    Patch4(0x009F7078 + 1, 1080);

    // CUIToolTip::MakeLayer - maximum bounds for CUIToolTip
    PatchJmp(CUIToolTip__MakeLayer_jmp1, &CUIToolTip__MakeLayer_hook1);
    PatchJmp(CUIToolTip__MakeLayer_jmp2, &CUIToolTip__MakeLayer_hook2);

    // CSlideNotice - sliding notice width
    Patch4(0x007E15BE + 1, 1920); // CSlideNotice::CSlideNotice
    Patch4(0x007E16BE + 1, 1920); // CSlideNotice::OnCreate
    Patch4(0x007E1E07 + 2, 1920); // CSlideNotice::SetMsg
}