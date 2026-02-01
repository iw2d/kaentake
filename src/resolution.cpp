#include "pch.h"
#include "hook.h"
#include "wvs/wnd.h"
#include "wvs/ctrlwnd.h"
#include "wvs/stage.h"
#include "wvs/field.h"
#include "wvs/rtti.h"
#include "wvs/util.h"
#include "ztl/ztl.h"

#include <windows.h>
#include <strsafe.h>
#include <intrin.h>

#define STATUS_BAR_ORIGIN CWnd::UIOrigin::Origin_CB
#define SCREEN_WIDTH_MAX  1920
#define SCREEN_HEIGHT_MAX 1080


static ZRef<CCtrlComboBox> g_cbResolution;
static int g_nResolution = 0;
static int g_nScreenWidth = 800;
static int g_nScreenHeight = 600;
static int g_nAdjustCenterY = 0;

void set_screen_resolution(int nResolution, bool bSave);

int get_screen_width() {
    return g_nScreenWidth;
}

int get_screen_height() {
    return g_nScreenHeight;
}

int get_adjust_cy() {
    return g_nAdjustCenterY;
}

void get_default_position(int nUIType, int* pnDefaultX, int* pnDefaultY) {
    int nDefaultX;
    int nDefaultY;
    switch (nUIType) {
    case 4:
        nDefaultX = 8;
        nDefaultY = 8;
        break;
    case 8:
        nDefaultX = 500;
        nDefaultY = 50;
        break;
    case 9:
    case 22:
        nDefaultX = (nUIType == 9) ? 250 : 500;
        nDefaultY = 100;
        break;
    case 14:
        nDefaultX = 600;
        nDefaultY = 35;
        break;
    case 15:
        nDefaultX = 730;
        nDefaultY = 400;
        break;
    case 18:
        nDefaultX = 11;
        nDefaultY = 24;
        break;
    case 20:
        nDefaultX = 720;
        nDefaultY = 80;
        break;
    case 23:
    case 31:
    case 33:
        nDefaultX = 100;
        nDefaultY = 100;
        break;
    case 24:
    case 25:
    case 26:
    case 27:
    case 29:
    case 32:
        nDefaultX = 244;
        nDefaultY = 105;
        break;
    case 30:
        nDefaultX = 769;
        nDefaultY = 343;
        break;
    default:
        nDefaultX = 8 * (3 * nUIType + 3);
        nDefaultY = nDefaultX;
        break;
    }
    if (pnDefaultX) {
        *pnDefaultX = nDefaultX;
    }
    if (pnDefaultY) {
        *pnDefaultY = nDefaultY;
    }
}


static auto set_stage = reinterpret_cast<void(__cdecl*)(CStage*, void*)>(0x00777347);
void __cdecl set_stage_hook(CStage* pStage, void* pParam) {
    // CField::ms_RTTI_CField - change resolution before set_stage
    if (pStage && pStage->IsKindOf(reinterpret_cast<const CRTTI*>(0x00BED758))) {
        set_screen_resolution(g_nResolution, 0);
        set_stage(pStage, pParam);
        return;
    }
    set_stage(pStage, pParam);
    // !CInterStage::ms_RTTI_CInterStage - change resolution after set_stage
    if (pStage && !pStage->IsKindOf(reinterpret_cast<const CRTTI*>(0x00BED874))) {
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
    MEMBER_ARRAY_AT(int, 0xCC, m_nUIWnd_X, 34)
    MEMBER_ARRAY_AT(int, 0x154, m_nUIWnd_Y, 34)

    MEMBER_HOOK(void, 0x0049F0B5, GetUIWndPos, int nUIType, int* x, int* y, int* op)
    MEMBER_HOOK(void, 0x0049D0B6, LoadCharacter, int nWorldID, unsigned int dwCharacterId)
    MEMBER_HOOK(void, 0x0049C441, LoadGlobal)
    MEMBER_HOOK(void, 0x0049C8E7, SaveGlobal)
    MEMBER_HOOK(void, 0x0049EA33, ApplySysOpt, void* pSysOpt, int bApplyVideo)

    int GetOpt_Int(int nType, const char* sKey, int nDefaultX, int nLowBound, int nHighBound) {
        return reinterpret_cast<int(__thiscall*)(CConfig*, int, const char*, int, int, int)>(0x0049EF65)(this, nType, sKey, nDefaultX, nLowBound, nHighBound);
    }
    void SetOpt_Int(int nType, const char* sKey, int nValue) {
        reinterpret_cast<void(__thiscall*)(CConfig*, int, const char*, int)>(0x0049EFB5)(this, nType, sKey, nValue);
    }
};

void CConfig::GetUIWndPos_hook(int nUIType, int* x, int* y, int* op) {
    CConfig::GetUIWndPos(this, nUIType, x, y, op);
    if (*x < -5 || *x > get_screen_width() - 6 || *y < -5 || *y > get_screen_height() - 6) {
        get_default_position(nUIType, x, y);
    }
}

void CConfig::LoadCharacter_hook(int nWorldID, unsigned int dwCharacterId) {
    CConfig::LoadCharacter(this, nWorldID, dwCharacterId);
    for (size_t i = 0; i < 34; ++i) {
        int nDefaultX;
        int nDefaultY;
        get_default_position(i, &nDefaultX, &nDefaultY);

        char sBuffer[1024];
        sprintf_s(sBuffer, 1024, "uiWndX%d", i);
        m_nUIWnd_X[i] = GetOpt_Int(GLOBAL_OPT, sBuffer, nDefaultX, -5, get_screen_width() - 6);
        sprintf_s(sBuffer, 1024, "uiWndY%d", i);
        m_nUIWnd_Y[i] = GetOpt_Int(GLOBAL_OPT, sBuffer, nDefaultY, -5, get_screen_height() - 6);
    }
}

void CConfig::LoadGlobal_hook() {
    CConfig::LoadGlobal(this);
    g_nResolution = GetOpt_Int(GLOBAL_OPT, "soScreenResolution", 0, 0, 4);
}

void CConfig::SaveGlobal_hook() {
    CConfig::SaveGlobal(this);
    SetOpt_Int(GLOBAL_OPT, "soScreenResolution", g_nResolution);
}

void CConfig::ApplySysOpt_hook(void* pSysOpt, int bApplyVideo) {
    CConfig::ApplySysOpt(this, pSysOpt, bApplyVideo);
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
    CUISysOpt::OnCreate(this, pData);

    CCtrlComboBox::CREATEPARAM paramComboBox;
    paramComboBox.nBackColor = 0xFFEEEEEE;
    paramComboBox.nBackFocusedColor = 0xFFA5A198;
    paramComboBox.nBorderColor = 0xFF999999;

    g_cbResolution = new CCtrlComboBox();
    g_cbResolution->CreateCtrl(this, 2000, 0, 76, 338, 166, 18, &paramComboBox);
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
    CUISysOpt::Destructor(this);
    g_cbResolution = nullptr;
}


class CInputSystem : public TSingleton<CInputSystem, 0x00BEC33C> {
public:
    MEMBER_AT(HWND, 0x0, m_hWnd)
    MEMBER_AT(IWzVector2DPtr, 0x9B0, m_pVectorCursor)
    MEMBER_HOOK(void, 0x0059A0CB, SetCursorVectorPos, int x, int y)
    MEMBER_HOOK(int, 0x0059A887, SetCursorPos, int x, int y)

    int GetCursorPos(POINT* lpPoint) {
        return ::GetCursorPos(lpPoint) && ::ScreenToClient(m_hWnd, lpPoint);
    }
};

void CInputSystem::SetCursorVectorPos_hook(int x, int y) {
    m_pVectorCursor->RelMove(x - get_screen_width() / 2, y - get_screen_height() / 2 - get_adjust_cy());
}

int CInputSystem::SetCursorPos_hook(int x, int y) {
    POINT pt;
    pt.x = zclamp(x, 0, get_screen_width());
    pt.y = zclamp(y, 0, get_screen_height());
    SetCursorVectorPos_hook(pt.x, pt.y);
    return ::ClientToScreen(m_hWnd, &pt) && ::SetCursorPos(pt.x, pt.y);
}


class CWndMan : public CWnd, public TSingleton<CWndMan, 0x00BEC20C> {
public:
    inline static ZList<CWnd*>& ms_lpWindow = *reinterpret_cast<ZList<CWnd*>*>(0x00BF1648);
    inline static IWzVector2DPtr ms_pOrgWindowEx[9];

    MEMBER_AT(IWzVector2DPtr, 0xDC, m_pOrgWindow)
    MEMBER_HOOK(void, 0x009E2C42, Constructor, HWND hWnd)
    MEMBER_HOOK(void, 0x009E3026, Destructor)
    MEMBER_HOOK(IWzVector2DPtr*, 0x0048BBA5, GetOrgWindow, IWzVector2DPtr* result)

    IWzVector2DPtr& GetOrgWindowEx(CWnd::UIOrigin org) {
        return ms_pOrgWindowEx[org];
    }
    void ResetOrgWindow() {
        auto pOrgWindow = CWndMan::GetInstance()->m_pOrgWindow;
        pOrgWindow->origin = static_cast<IUnknown*>(get_gr()->center);
        pOrgWindow->RelMove(-(get_screen_width() / 2), -(get_screen_height() / 2) - get_adjust_cy());
        for (int i = 0; i < CWnd::UIOrigin::Origin_NUM; ++i) {
            int nX = -(get_screen_width() / 2);
            if (i % 3 == 1) {
                nX += (get_screen_width() - 800) / 2;
            } else if (i % 3 == 2) {
                nX += (get_screen_width() - 800);
            }
            int nY = -(get_screen_height() / 2) - get_adjust_cy();
            if (i / 3 == 1) {
                nY += (get_screen_height() - 600) / 2;
            } else if (i / 3 == 2) {
                nY += (get_screen_height() - 600);
            }
            ms_pOrgWindowEx[i]->origin = static_cast<IUnknown*>(get_gr()->center);
            ms_pOrgWindowEx[i]->RelMove(nX, nY);
        }
    }
};

void CWndMan::Constructor_hook(HWND hWnd) {
    CWndMan::Constructor(this, hWnd);
    for (int i = 0; i < CWnd::UIOrigin::Origin_NUM; ++i) {
        PcCreateObject<IWzVector2DPtr>(L"Shape2D#Vector2D", ms_pOrgWindowEx[i], nullptr);
    }
    ResetOrgWindow();
}

void CWndMan::Destructor_hook() {
    CWndMan::Destructor(this);
    for (int i = 0; i < CWnd::UIOrigin::Origin_NUM; ++i) {
        ms_pOrgWindowEx[i] = nullptr;
    }
}


IWzVector2DPtr* CWndMan::GetOrgWindow_hook(IWzVector2DPtr* result) {
    auto ret = reinterpret_cast<uintptr_t>(_ReturnAddress());
    switch (ret) {
    case 0x00533B77: // CField::ShowMobHPTag
    case 0x005555B7: // CField_Dojang::OnClock
    case 0x0058C91C: // CFloatNotice::CreateFloatNotice
    case 0x006CD787: // CNoticeQuestProgress::CNoticeQuestProgress
        result->GetInterfacePtr() = GetOrgWindowEx(CWnd::UIOrigin::Origin_CT);
        break;
    case 0x009603F2: // CUserLocal::DrawCombo
        result->GetInterfacePtr() = GetOrgWindowEx(CWnd::UIOrigin::Origin_RT);
        break;
    case 0x0053502B: // CField::ShowScreenEffect
        result->GetInterfacePtr() = GetOrgWindowEx(CWnd::UIOrigin::Origin_CC);
        break;
    case 0x0089AF82: // CUIScreenMsg::CUIScreenMsg
    case 0x008DEB75: // CUIStatusBar::FlashHPBar
    case 0x008DEE11: // CUIStatusBar::FlashMPBar
        result->GetInterfacePtr() = GetOrgWindowEx(STATUS_BAR_ORIGIN);
        break;
    default:
        if (ret >= 0x008D01B2 && ret <= 0x008D3ADF) { // CUIStatusBar::OnCreate
            result->GetInterfacePtr() = GetOrgWindowEx(STATUS_BAR_ORIGIN);
        } else if (ret >= 0x00554005 && ret <= 0x0055478F) { // CField_Dojang::Init
            result->GetInterfacePtr() = GetOrgWindowEx(Origin_CT);
        } else {
            result->GetInterfacePtr() = m_pOrgWindow;
        }
        break;
    }
    result->AddRef();
    return result;
}

void CWnd::CreateWnd_hook(int l, int t, int w, int h, int z, int bScreenCoord, void* pData, int bSetFocus) {
    CWnd::CreateWnd(this, l, t, w, h, z, bScreenCoord, pData, bSetFocus);
    if (!bScreenCoord) {
        return;
    }
    auto ret = reinterpret_cast<uintptr_t>(_ReturnAddress());
    switch (ret) {
    case 0x005362BC: // CField::OnClock(CField*, int)
    case 0x0053638B: // CField::OnClock(CField*, int)
    case 0x00545D24: // CField_Battlefield::OnClock(CField*, int)
    case 0x0056042E: // CField_Massacre::OnClock(CField*, int)
    case 0x00578B30: // CField_SpaceGAGA::OnClock(CField*, int)
    case 0x00A24D15: // CWvsContext::SetEventTimer(CWvsContext*, int)
        m_pLayer->origin = static_cast<IUnknown*>(CWndMan::GetInstance()->GetOrgWindowEx(CWnd::UIOrigin::Origin_CT));
        return;
    case 0x0045A5EF: // CAvatarMegaphone::CAvatarMegaphone
        m_pLayer->origin = static_cast<IUnknown*>(CWndMan::GetInstance()->GetOrgWindowEx(CWnd::UIOrigin::Origin_RT));
        return;
    case 0x004EDAEB: // CDialog::CreateDlg(CDialog*, int, int, int, void*)
    case 0x004EDB9A: // CDialog::CreateDlg(CDialog*, const wchar_t*, int, void*)
    case 0x004EDAB3: // CDialog::CreateDlg(CDialog*, int, int, int, int, int, int, void*)
        m_pLayer->origin = static_cast<IUnknown*>(CWndMan::GetInstance()->GetOrgWindowEx(CWnd::UIOrigin::Origin_CC));
        return;
    case 0x0051FA03: // CFadeWnd::CreateFadeWnd
    case 0x008CFD65: // CUIStatusBar::CUIStatusBar
        m_pLayer->origin = static_cast<IUnknown*>(CWndMan::GetInstance()->GetOrgWindowEx(STATUS_BAR_ORIGIN));
        return;
    }
}

static auto CWnd__PreCreateWnd = reinterpret_cast<void(__thiscall*)(CWnd*, int, int, int, int, int, int, void*)>(0x009DE7FB);
void __fastcall CWnd__PreCreateWnd_hook(CWnd* pThis, void* _EDX, int l, int t, int w, int h, int z, int bScreenCoord, void* pData) {
    auto ret = reinterpret_cast<uintptr_t>(_ReturnAddress());
    // CEngageDlg::PreCreateWnd || CUtilDlg::PreCreateWnd
    if (ret == 0x00515D9C || ret == 0x00991873) {
        l = l + (get_screen_width() - 800) / 2;
        t = t + (get_screen_height() - 600) / 2;
    }
    CWnd__PreCreateWnd(pThis, l, t, w, h, z, bScreenCoord, pData);
}

static auto CWnd__OnMoveWnd = reinterpret_cast<void(__thiscall*)(CWnd*, int, int)>(0x009DEB57);

void MoveWndToAbsPos(CWnd* pWnd, int l, int t) {
    IWzVector2DPtr pAbsOrigin = CWndMan::GetInstance()->m_pOrgWindow;
    IWzVector2DPtr pWndOrigin = static_cast<IUnknown*>(pWnd->m_pLayer->origin);
    int nOffsetX = pAbsOrigin->x - pWndOrigin->x;
    int nOffsetY = pAbsOrigin->y - pWndOrigin->y;
    pWnd->m_pLayer->RelMove(l + nOffsetX, t + nOffsetY);
}

void __fastcall CWnd__OnMoveWnd_hook(CWnd* pThis, void* _EDX, int l, int t) {
    int nLeft = pThis->GetAbsLeft();
    int nTop = pThis->GetAbsTop();
    int nWidth = pThis->m_pLayer->width;
    int nHeight = pThis->m_pLayer->height;
    // Save m_ptCursorRel
    POINT pt;
    CInputSystem::GetInstance()->GetCursorPos(&pt);
    POINT ptRel;
    ptRel.x = pt.x - nLeft;
    ptRel.y = pt.y - nTop;
    if (pThis->m_ptCursorRel.x == -1 && pThis->m_ptCursorRel.y == -1) {
        pThis->m_ptCursorRel.x = ptRel.x;
        pThis->m_ptCursorRel.y = ptRel.y;
    }
    // Iterate ZList<CWnd*> for window snapping
    RECT rcThis;
    SetRect(&rcThis, nLeft, nTop, nLeft + nWidth, nTop + nHeight);
    auto pos = CWndMan::ms_lpWindow.GetHeadPosition();
    while (pos) {
        auto pNext = CWndMan::ms_lpWindow.GetNext(pos);
        if (pNext == pThis || pThis->IsMyAddOn(pNext) ||
                pNext == reinterpret_cast<CWnd*>(0x00BEC208)) { // TSingleton<CUIStatusBar>::ms_pInstance
            continue;
        }
        int nNextLeft = pNext->GetAbsLeft();
        int nNextTop = pNext->GetAbsTop();
        int nNextWidth = pNext->m_pLayer->width;
        int nNextHeight = pNext->m_pLayer->height;
        RECT rcNext, rcIntersect;
        SetRect(&rcNext, nNextLeft - 10, nNextTop - 10, nNextLeft + nNextWidth + 10, nNextTop + nNextHeight + 10);
        if (!IntersectRect(&rcIntersect, &rcThis, &rcNext)) {
            continue;
        }
        if (abs(nLeft - nNextLeft - nNextWidth) <= 10) {
            MoveWndToAbsPos(pThis, nNextLeft + nNextWidth, nTop);
        }
        if (abs(nLeft - nNextLeft + nWidth) <= 10) {
            MoveWndToAbsPos(pThis, nNextLeft - nWidth, nTop);
        }
        if (abs(nTop - nNextTop - nNextHeight) <= 10) {
            MoveWndToAbsPos(pThis, nLeft, nNextTop + nNextHeight);
        }
        if (abs(nTop - nNextTop + nHeight) <= 10) {
            MoveWndToAbsPos(pThis, nLeft, nNextTop - nHeight);
        }
    }
    // Window snapping to screen border
    if (abs(nLeft) <= 10) {
        MoveWndToAbsPos(pThis, 0, nTop);
    }
    if (abs(nTop) <= 10) {
        MoveWndToAbsPos(pThis, nLeft, 0);
    }
    if (abs(nLeft + nWidth - get_screen_width()) <= 10) {
        MoveWndToAbsPos(pThis, get_screen_width() - nWidth, nTop);
    }
    if (abs(nTop + nHeight - get_screen_height()) <= 10) {
        MoveWndToAbsPos(pThis, nLeft, get_screen_height() - nHeight);
    }
    // Handle m_ptCursorRel
    if (abs(pThis->m_ptCursorRel.x - ptRel.x) > 15) {
        MoveWndToAbsPos(pThis, pt.x - pThis->m_ptCursorRel.x, nTop);
    }
    if (abs(pThis->m_ptCursorRel.y - ptRel.y) > 15) {
        MoveWndToAbsPos(pThis, nLeft, pt.y - pThis->m_ptCursorRel.y);
    }
}


class CUtilDlgEx : public CWnd {
public:
    MEMBER_AT(int, 0x98, m_wndWidth)
    MEMBER_AT(int, 0x9C, m_wndHeight)
    MEMBER_HOOK(void, 0x009A3E38, CreateUtilDlgEx)
};

static RECT& sRectQuestDlg = *reinterpret_cast<RECT*>(0x00BE2DF0);

void CUtilDlgEx::CreateUtilDlgEx_hook() {
    int nLeft = zclamp<int>(sRectQuestDlg.left - m_wndWidth / 2, 0, get_screen_width());
    int nTop = zclamp<int>(sRectQuestDlg.top - m_wndHeight / 2, 0, get_screen_height());
    // CDialog::CreateDlg(this, nLeft, nTop, m_wndWidth, m_wndHeight, 10, 1, 0);
    CWnd::CreateWnd(this, nLeft, nTop, m_wndWidth, m_wndHeight, 10, 1, nullptr, 1);
}


class CUIToolTip {
public:
    MEMBER_AT(int, 0x8, m_nHeight)
    MEMBER_AT(int, 0xC, m_nWidth)
    MEMBER_AT(IWzGr2DLayerPtr, 0x10, m_pLayer)
    MEMBER_HOOK(IWzCanvasPtr*, 0x008F3141, MakeLayer, IWzCanvasPtr* result, int nLeft, int nTop, int bDoubleOutline, int bLogin, int bCharToolTip, unsigned int uColor)
};

IWzCanvasPtr* CUIToolTip::MakeLayer_hook(IWzCanvasPtr* result, int nLeft, int nTop, int bDoubleOutline, int bLogin, int bCharToolTip, unsigned int uColor) {
    CUIToolTip::MakeLayer(this, result, nLeft, nTop, bDoubleOutline, bLogin, bCharToolTip, uColor);
    if (!bCharToolTip) {
        if (nLeft < 0) {
            nLeft = 0;
        }
        if (nTop < 0) {
            nTop = 0;
        }
        int nBoundX = get_screen_width() - 1;
        if (nLeft + m_nWidth > nBoundX) {
            nLeft = nBoundX - m_nWidth;
        }
        int nBoundY = get_screen_height() - 1;
        if (nTop + m_nHeight > nBoundY) {
            nTop = nBoundY - m_nHeight;
        }
        m_pLayer->RelMove(nLeft, nTop);
    }
    return result;
}


class CTemporaryStatView {
public:
    struct TEMPORARY_STAT : public ZRefCounted {
        unsigned char pad0[0x40 - sizeof(ZRefCounted)];
        MEMBER_AT(int, 0x1C, nType)
        MEMBER_AT(IWzGr2DLayerPtr, 0x28, pLayer)
        MEMBER_AT(IWzGr2DLayerPtr, 0x2C, pLayerShadow)
    };
    MEMBER_AT(ZList<ZRef<TEMPORARY_STAT>>, 0x4, m_lTemporaryStat)
    MEMBER_HOOK(void, 0x007B2BB0, AdjustPosition)
};

void CTemporaryStatView::AdjustPosition_hook() {
    int nOffsetX = (get_screen_width() / 2) - 3 + (-32 * m_lTemporaryStat.GetCount());
    int nOffsetY = (get_screen_height() / 2) + get_adjust_cy() - 23;
    auto pos = m_lTemporaryStat.GetHeadPosition();
    while (pos) {
        auto pNext = m_lTemporaryStat.GetNext(pos);
        pNext->pLayer->RelMove((32 - pNext->pLayer->width) / 2 + nOffsetX, (32 - pNext->pLayer->height) / 2 - nOffsetY);
        pNext->pLayerShadow->RelMove((32 - pNext->pLayerShadow->width) / 2 + nOffsetX, (32 - pNext->pLayerShadow->height) / 2 - nOffsetY);
        nOffsetX += 32;
    }
}


class CWvsContext : public TSingleton<CWvsContext, 0x00BE7918> {
public:
    MEMBER_AT(CTemporaryStatView, 0x2EA8, m_temporaryStatView)
};

class CWvsPhysicalSpace2D : public TSingleton<CWvsPhysicalSpace2D, 0x00BEBFA0> {
public:
    MEMBER_AT(RECT, 0x24, m_rcMBR)
};

void CMapLoadable::RestoreViewRange_hook() {
    auto pSpace2D = CWvsPhysicalSpace2D::GetInstance();
    m_rcViewRange.left = get_int32(m_pPropFieldInfo->item[L"VRLeft"], pSpace2D->m_rcMBR.left - 20) + get_screen_width() / 2;
    m_rcViewRange.top = get_int32(m_pPropFieldInfo->item[L"VRTop"], pSpace2D->m_rcMBR.top - 60) + get_screen_height() / 2;
    m_rcViewRange.right = get_int32(m_pPropFieldInfo->item[L"VRRight"], pSpace2D->m_rcMBR.right + 20) - get_screen_width() / 2;
    m_rcViewRange.bottom = get_int32(m_pPropFieldInfo->item[L"VRBottom"], pSpace2D->m_rcMBR.bottom + 190) - get_screen_height() / 2;
    if (m_rcViewRange.right - m_rcViewRange.left <= 0) {
        int mid = (m_rcViewRange.left + m_rcViewRange.right) / 2;
        m_rcViewRange.left = mid;
        m_rcViewRange.right = mid;
    }
    if (m_rcViewRange.bottom - m_rcViewRange.top <= 0) {
        int mid = (m_rcViewRange.top + m_rcViewRange.bottom) / 2;
        m_rcViewRange.top = mid;
        m_rcViewRange.bottom = mid;
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

HRESULT __stdcall CMapLoadable__raw_WrapClip_hook(IWzVector2D* pThis, VARIANT pOrigin, int nWrapLeft, int nWrapTop, unsigned int uWrapWidth, unsigned int uWrapHeight, VARIANT bClip) {
    nWrapLeft = nWrapLeft + 400 - (SCREEN_WIDTH_MAX / 2);
    nWrapTop = nWrapTop + 300 - (SCREEN_HEIGHT_MAX / 2) - ((SCREEN_HEIGHT_MAX - 600) / 2);
    uWrapWidth = uWrapWidth - 800 + SCREEN_WIDTH_MAX;
    uWrapHeight = uWrapHeight - 600 + SCREEN_HEIGHT_MAX;
    return pThis->raw_WrapClip(pOrigin, nWrapLeft, nWrapTop, uWrapWidth, uWrapHeight, bClip);
}

HRESULT __stdcall CField_LimitedView__raw_Copy_hook(IWzCanvas* pThis, int nDstLeft, int nDstTop, IWzCanvas* pSource, VARIANT nAlpha) {
    nDstLeft = nDstLeft + (SCREEN_WIDTH_MAX / 2) - 400;
    nDstTop = nDstTop + (SCREEN_HEIGHT_MAX / 2) - 300 + ((SCREEN_HEIGHT_MAX - 600) / 2);
    return pThis->raw_Copy(nDstLeft, nDstTop, pSource, nAlpha);
}

HRESULT __fastcall CField_LimitedView__CopyEx_hook(IWzCanvas* pThis, void* _EDX, int nDstLeft, int nDstTop, IWzCanvas* pSource, CANVAS_ALPHATYPE nAlpha, int nWidth, int nHeight, int nSrcLeft, int nSrcTop, int nSrcWidth, int nSrcHeight, const Ztl_variant_t& pAdjust) {
    nDstLeft = nDstLeft + (SCREEN_WIDTH_MAX / 2) - 400;
    nDstTop = nDstTop + (SCREEN_HEIGHT_MAX / 2) - 300 + ((SCREEN_HEIGHT_MAX - 600) / 2);
    return pThis->CopyEx(nDstLeft, nDstTop, pSource, nAlpha, nWidth, nHeight, nSrcLeft, nSrcTop, nSrcWidth, nSrcHeight, pAdjust);
}


class CUIMiniMap : public CUIWnd, public TSingleton<CUIMiniMap, 0x00BED788> {
};

int __stdcall CField__ShowMobHPTag_hook1() {
    if (CUIMiniMap::IsInstantiated() && g_nResolution == 0) {
        return CUIMiniMap::GetInstance()->m_width;
    }
    return 0;
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
        if (FAILED(gr->screenResolution(nScreenWidth, nScreenHeight))) {
            return;
        }
        g_nScreenWidth = nScreenWidth;
        g_nScreenHeight = nScreenHeight;
        g_nAdjustCenterY = (g_nScreenHeight - 600) / 2;
        gr->AdjustCenter(0, -g_nAdjustCenterY);
        if (CWndMan::IsInstantiated()) {
            CWndMan::GetInstance()->ResetOrgWindow();
            // Adjust CUtilDlgEx position
            sRectQuestDlg.top = get_screen_height() / 2;
            sRectQuestDlg.left = get_screen_width() / 2;
        }
        if (CWvsContext::IsInstantiated()) {
            CWvsContext::GetInstance()->m_temporaryStatView.AdjustPosition_hook();
        }
        CField* field = get_field();
        if (field) {
            field->RestoreViewRange_hook();
            // CMapLoadable::ReloadBack
            reinterpret_cast<void(__thiscall*)(CMapLoadable*)>(0x00644491)(field);
        }
    }
    if (bSave) {
        g_nResolution = nResolution;
    }
}


static auto CScreenShot__SaveFullScreenToJpg = reinterpret_cast<void(__cdecl*)(HWND hWnd, ZXString<char> sFileName)>(0x00744D77);
void __cdecl CScreenShot__SaveFullScreenToJpg_hook(HWND hWnd, ZXString<char> sFileName) {
    DEBUG_MESSAGE("CScreenShot::SaveFullScreenToJpg %s", sFileName);
    IWzGr2D_DX9Ptr gr = get_gr()->GetInner();
    gr->TakeScreenShot(static_cast<char*>(sFileName), 0x1); // SFF_JPG
}


class CWzGr2D_DX9 {
public:
    MEMBER_AT(int, 0x54, m_bWindow)
    MEMBER_AT(int, 0x98, m_nScreenLeft)
    MEMBER_AT(int, 0x9C, m_nScreenTop)
    MEMBER_AT(HWND, 0xA8, m_hWnd)
};

static void* CWzGr2D_DX9__RepositionWindow;
void __fastcall CWzGr2D_DX9__RepositionWindow_hook(CWzGr2D_DX9* pThis, void* _EDX) {
    LONG lStyle = GetWindowLongA(pThis->m_hWnd, GWL_STYLE);
    if ((lStyle & WS_CHILD) != 0) {
        return;
    }
    UINT uFlags;
    if (pThis->m_bWindow) {
        lStyle = lStyle & 0x7F35FFFF | 0xCA0000;
        uFlags = 0x29;
    } else {
        lStyle = 0x80080000;
        uFlags = 0x2B;
    }
    SetWindowLongA(pThis->m_hWnd, GWL_STYLE, lStyle);
    SetWindowPos(pThis->m_hWnd, pThis->m_bWindow ? HWND_NOTOPMOST : HWND_TOPMOST, pThis->m_nScreenLeft, pThis->m_nScreenTop, 0, 0, uFlags);
}


void AttachResolutionMod() {
    ATTACH_HOOK(set_stage, set_stage_hook);
    ATTACH_HOOK(CConfig::GetUIWndPos, CConfig::GetUIWndPos_hook);
    ATTACH_HOOK(CConfig::LoadCharacter, CConfig::LoadCharacter_hook);
    ATTACH_HOOK(CConfig::LoadGlobal, CConfig::LoadGlobal_hook);
    ATTACH_HOOK(CConfig::SaveGlobal, CConfig::SaveGlobal_hook);
    ATTACH_HOOK(CConfig::ApplySysOpt, CConfig::ApplySysOpt_hook);
    ATTACH_HOOK(CUISysOpt::OnCreate, CUISysOpt::OnCreate_hook);
    ATTACH_HOOK(CUISysOpt::Destructor, CUISysOpt::Destructor_hook);
    Patch4(0x009945BC + 1, 372);               // CUISysOpt::OnCreate - shift ok/cancel buttons
    Patch4(0x009F7078 + 1, SCREEN_HEIGHT_MAX); // CWvsApp::CreateWndManager - nHeight
    Patch4(0x009F707D + 1, SCREEN_WIDTH_MAX);  // CWvsApp::CreateWndManager - nWidth

    ATTACH_HOOK(CInputSystem::SetCursorVectorPos, CInputSystem::SetCursorVectorPos_hook);
    ATTACH_HOOK(CInputSystem::SetCursorPos, CInputSystem::SetCursorPos_hook);
    ATTACH_HOOK(CWndMan::Constructor, CWndMan::Constructor_hook);
    ATTACH_HOOK(CWndMan::Destructor, CWndMan::Destructor_hook);
    ATTACH_HOOK(CWndMan::GetOrgWindow, CWndMan::GetOrgWindow_hook);
    ATTACH_HOOK(CWnd::CreateWnd, CWnd::CreateWnd_hook);
    ATTACH_HOOK(CWnd__PreCreateWnd, CWnd__PreCreateWnd_hook);
    ATTACH_HOOK(CWnd__OnMoveWnd, CWnd__OnMoveWnd_hook);

    // CUtilDlgEx::CreateUtilDlgEx - adjust for screen bounds
    ATTACH_HOOK(CUtilDlgEx::CreateUtilDlgEx, CUtilDlgEx::CreateUtilDlgEx_hook);

    // CUIToolTip::MakeLayer - handle maximum bounds for CUIToolTip
    ATTACH_HOOK(CUIToolTip::MakeLayer, CUIToolTip::MakeLayer_hook);

    // CTemporaryStatView - reposition buff display
    ATTACH_HOOK(CTemporaryStatView::AdjustPosition, CTemporaryStatView::AdjustPosition_hook);

    // CMapLoadable - handle view range
    ATTACH_HOOK(CMapLoadable::RestoreViewRange, CMapLoadable::RestoreViewRange_hook);
    PatchJmp(CMapLoadable__MakeGrid_jmp, &CMapLoadable__MakeGrid_hook);

    // CMapLoadable::TransientLayer_Weather - weather effects
    PatchCall(0x0064106B, CMapLoadable__raw_WrapClip_hook, 6);
    Patch4(0x0064043E + 1, SCREEN_WIDTH_MAX / 2);
    Patch4(0x00640443 + 1, SCREEN_HEIGHT_MAX / 2);
    Patch4(0x00640599 + 2, SCREEN_WIDTH_MAX / 2 - 10);
    Patch4(0x006405BA + 2, SCREEN_HEIGHT_MAX - 10);
    Patch4(0x00640606 + 1, SCREEN_WIDTH_MAX);
    Patch4(0x00640618 + 1, SCREEN_HEIGHT_MAX);
    Patch4(0x00640626 + 1, SCREEN_WIDTH_MAX / 2);
    Patch4(0x00640639 + 1, SCREEN_WIDTH_MAX);
    Patch4(0x0064064B + 1, SCREEN_HEIGHT_MAX);
    Patch4(0x00640656 + 2, -SCREEN_WIDTH_MAX / 2);
    Patch4(0x006406C3 + 1, SCREEN_WIDTH_MAX);
    Patch4(0x006406D5 + 1, SCREEN_HEIGHT_MAX);
    Patch4(0x006406FA + 2, SCREEN_HEIGHT_MAX / 2);

    // CField_LimitedView::Init
    Patch4(0x0055B808 + 1, SCREEN_HEIGHT_MAX);                                      // m_pCanvasDark->raw_Create - uHeight
    Patch4(0x0055B80D + 1, SCREEN_WIDTH_MAX);                                       // m_pCanvasDark->raw_Create - uWidth
    Patch4(0x0055B884 + 1, SCREEN_HEIGHT_MAX);                                      // m_pCanvasDark->raw_DrawRectangle - uHeight
    Patch4(0x0055BB2F + 1, -SCREEN_HEIGHT_MAX / 2 - (SCREEN_HEIGHT_MAX - 600) / 2); // m_pLayerDark->raw_RelMove - nY
    Patch4(0x0055BB35 + 1, -SCREEN_WIDTH_MAX / 2);                                  // m_pLayerDark->raw_RelMove - nX
    // CField_LimitedView::DrawViewRange
    PatchCall(0x0055BEFE, &CField_LimitedView__raw_Copy_hook, 6);
    PatchCall(0x0055C08E, &CField_LimitedView__CopyEx_hook);
    PatchCall(0x0055C1DD, &CField_LimitedView__CopyEx_hook);

    // CSlideNotice - sliding notice width
    Patch4(0x007E15BE + 1, SCREEN_WIDTH_MAX); // CSlideNotice::CSlideNotice
    Patch4(0x007E16BE + 1, SCREEN_WIDTH_MAX); // CSlideNotice::OnCreate
    Patch4(0x007E1E07 + 2, SCREEN_WIDTH_MAX); // CSlideNotice::SetMsg

    // CField::ShowMobHPTag - boss hp bar position
    PatchCall(0x00533705, &CField__ShowMobHPTag_hook1, 15); // nLeft

    // CScreenShot::SaveFullScreenToJpg
    ATTACH_HOOK(CScreenShot__SaveFullScreenToJpg, CScreenShot__SaveFullScreenToJpg_hook);

    // Gr2D_DX9.dll - reimplement window repositioning function as it does not account for multiple monitors
    CWzGr2D_DX9__RepositionWindow = GetAddressByPattern("Gr2D_DX9.dll", "56 8B F1 8B 86 A8 00 00 00");
    ATTACH_HOOK(CWzGr2D_DX9__RepositionWindow, CWzGr2D_DX9__RepositionWindow_hook);
}