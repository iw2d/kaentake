#include "hook.h"
#include "wvs/wnd.h"
#include "wvs/ctrlwnd.h"
#include "wvs/stage.h"
#include "wvs/field.h"
#include "wvs/rtti.h"
#include "wvs/util.h"
#include "ztl/zalloc.h"
#include "ztl/zcoll.h"
#include "ztl/zcom.h"
#include "ztl/ztl.h"
#include "ztl/tsingleton.h"
#include <windows.h>
#include <strsafe.h>

#define SCREEN_WIDTH_MAX  1920
#define SCREEN_HEIGHT_MAX 1080


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

int get_adjust_dx() {
    return -(g_nScreenWidth - 800) / 2;
}
int get_adjust_dy() {
    return -(g_nScreenHeight - 600) / 2 - g_nAdjustCenterY;
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
        *pnDefaultX = nDefaultX + get_adjust_dx();
    }
    if (pnDefaultY) {
        *pnDefaultY = nDefaultY + get_adjust_dy();
    }
}

void set_screen_resolution(int nResolution, bool bSave);


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

void CConfig::LoadCharacter_hook(int nWorldID, unsigned int dwCharacterId) {
    CConfig::LoadCharacter(this, nWorldID, dwCharacterId);
    for (size_t i = 0; i < 34; ++i) {
        int nDefaultX;
        int nDefaultY;
        get_default_position(i, &nDefaultX, &nDefaultY);

        char sBuffer[1024];
        sprintf_s(sBuffer, 1024, "uiWndX%d", i);
        m_nUIWnd_X[i] = GetOpt_Int(GLOBAL_OPT, sBuffer, nDefaultX, get_adjust_dx() - 5, get_screen_width() + get_adjust_dx() - 6);
        sprintf_s(sBuffer, 1024, "uiWndY%d", i);
        m_nUIWnd_Y[i] = GetOpt_Int(GLOBAL_OPT, sBuffer, nDefaultY, get_adjust_dy() - 5, get_screen_height() + get_adjust_dy() - 6);
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

    MEMBER_AT(IWzVector2DPtr, 0xDC, m_pOrgWindow)
    MEMBER_HOOK(void, 0x009E311B, GetCursorPos, POINT* lpPoint, int bField)
    MEMBER_HOOK(IUIMsgHandler*, 0x009E42B2, GetHandlerFromPoint, int x, int y)
};

void CWndMan::GetCursorPos_hook(POINT* lpPoint, int bField) {
    if (CInputSystem::IsInstantiated()) {
        CInputSystem::GetInstance()->GetCursorPos(lpPoint);
        lpPoint->x += get_adjust_dx();
        lpPoint->y += get_adjust_dy();
        if (bField) {
            lpPoint->x += m_pOrgWindow->x;
            lpPoint->y += m_pOrgWindow->y;
        }
    }
}

IUIMsgHandler* CWndMan::GetHandlerFromPoint_hook(int x, int y) {
    IUIMsgHandler* pHandler = CWndMan::GetHandlerFromPoint(this, x, y);
    if (!pHandler && x >= get_adjust_dx() && x < get_screen_width() + get_adjust_dx() && y >= get_adjust_dy() && y < get_screen_height() + get_adjust_dy()) {
        return CWndMan::GetInstance();
    }
    return pHandler;
}


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


static auto CWnd__OnMoveWnd = reinterpret_cast<void(__thiscall*)(CWnd*, int, int)>(0x009DEB57);
void __fastcall CWnd__OnMoveWnd_hook(CWnd* pThis, void* _EDX, int l, int t) {
    CWnd__OnMoveWnd(pThis, l, t);
    int nAbsLeft = pThis->GetAbsLeft();
    int nAbsTop = pThis->GetAbsTop();
    if (abs(nAbsLeft - get_adjust_dx()) <= 10) {
        pThis->m_pLayer->lt->RelMove(get_adjust_dx(), nAbsTop);
    }
    if (abs(nAbsTop - get_adjust_dy()) <= 10) {
        pThis->m_pLayer->lt->RelMove(nAbsLeft, get_adjust_dy());
    }
    int nWidth = pThis->m_pLayer->width;
    int nBoundX = get_screen_width() + get_adjust_dx();
    if (abs(nAbsLeft + nWidth - nBoundX) <= 10) {
        pThis->m_pLayer->lt->RelMove(nBoundX - nWidth, nAbsTop);
    }
    int nHeight = pThis->m_pLayer->height;
    int nBoundY = get_screen_height() + get_adjust_dy();
    if (abs(nAbsTop + nHeight - nBoundY) <= 10) {
        pThis->m_pLayer->lt->RelMove(nAbsLeft, nBoundY - nHeight);
    }
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
        if (nLeft < get_adjust_dx()) {
            nLeft = get_adjust_dx();
        }
        if (nTop < get_adjust_dy()) {
            nTop = get_adjust_dy();
        }
        int nBoundX = get_screen_width() + get_adjust_dx() - 1;
        if (nLeft + m_nWidth > nBoundX) {
            nLeft = nBoundX - m_nWidth;
        }
        int nBoundY = get_screen_height() + get_adjust_dy() - 1;
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
    MEMBER_HOOK(int, 0x007B2E58, ShowToolTip, CUIToolTip& uiToolTip, POINT* ptCursor, int rx, int ry)
    MEMBER_HOOK(void, 0x007B3055, FindIcon, POINT* ptCursor, int& nType, int& nID)
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

int CTemporaryStatView::ShowToolTip_hook(CUIToolTip& uiToolTip, POINT* ptCursor, int rx, int ry) {
    ptCursor->x += get_adjust_dx();
    ptCursor->y -= get_adjust_dy();
    return CTemporaryStatView::ShowToolTip(this, uiToolTip, ptCursor, rx, ry);
}

void CTemporaryStatView::FindIcon_hook(POINT* ptCursor, int& nType, int& nID) {
    ptCursor->x += get_adjust_dx();
    ptCursor->y -= get_adjust_dy();
    CTemporaryStatView::FindIcon(this, ptCursor, nType, nID);
}


HRESULT __stdcall CAvatarMegaphone__raw_RelMove_hook1(IWzGr2DLayer* pThis, int nX, int nY, VARIANT nTime, VARIANT nType) {
    nX = get_screen_width() + get_adjust_dx();
    nY = get_adjust_dy();
    return pThis->raw_RelMove(nX, nY, nTime, nType);
}

HRESULT __stdcall CAvatarMegaphone__raw_RelMove_hook2(IWzGr2DLayer* pThis, int nX, int nY, VARIANT nTime, VARIANT nType) {
    nX = get_screen_width() + get_adjust_dx() - 225;
    nY = get_adjust_dy();
    return pThis->raw_RelMove(nX, nY, nTime, nType);
}


class CSlideNotice : public CWnd, public TSingleton<CSlideNotice, 0x00BF0DF4> {
public:
    MEMBER_HOOK(void, 0x007E16FE, SetMsg, void* sNotice) // ZXString<char>
};

void CSlideNotice::SetMsg_hook(void* sNotice) {
    CSlideNotice::SetMsg(this, sNotice);
    MoveWnd(get_adjust_dx(), get_adjust_dy());
}


class CWvsContext : public TSingleton<CWvsContext, 0x00BE7918> {
public:
    MEMBER_AT(CTemporaryStatView, 0x2EA8, m_temporaryStatView)
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
            if (CConfig::IsInstantiated()) {
                auto pos = CWndMan::ms_lpWindow.GetHeadPosition();
                while (pos) {
                    auto pNext = CWndMan::ms_lpWindow.GetNext(pos);
                    // (pNext == CUIStatusBar::GetInstance() || pNext->IsKindOf(CUIWnd::ms_RTTI_CUIWnd))
                    if (pNext == reinterpret_cast<CWnd*>(0x00BEC208) || !pNext->IsKindOf(reinterpret_cast<const CRTTI*>(0x00BF11DC))) {
                        continue;
                    }
                    // Check window position
                    if (pNext->GetAbsLeft() > get_adjust_dx() - 5 && pNext->GetAbsLeft() < get_screen_width() + get_adjust_dx() - 6 &&
                            pNext->GetAbsTop() > get_adjust_dy() - 5 && pNext->GetAbsTop() < get_screen_height() + get_adjust_dy() - 6) {
                        continue;
                    }
                    // Reposition window
                    auto pUIWnd = reinterpret_cast<CUIWnd*>(pNext);
                    int nUIType = pUIWnd->m_nUIType;
                    int nDefaultX;
                    int nDefaultY;
                    get_default_position(nUIType, &nDefaultX, &nDefaultY);
                    CConfig::GetInstance()->m_nUIWnd_X[nUIType] = nDefaultX;
                    CConfig::GetInstance()->m_nUIWnd_Y[nUIType] = nDefaultY;
                    pUIWnd->MoveWnd(nDefaultX, nDefaultY);
                }
            }
            if (CSlideNotice::IsInstantiated()) {
                CSlideNotice::GetInstance()->MoveWnd(get_adjust_dx(), get_adjust_dy());
            }
            if (CWvsContext::IsInstantiated()) {
                CWvsContext::GetInstance()->m_temporaryStatView.AdjustPosition_hook();
            }
            CField* field = get_field();
            if (field) {
                // CMapLoadable::OnEventChangeScreenResolution
                field->RestoreViewRange_hook();
                field->ReloadBack();
            }
        }
    }
    if (bSave) {
        g_nResolution = nResolution;
    }
}


void AttachResolutionMod() {
    ATTACH_HOOK(set_stage, set_stage_hook);
    ATTACH_HOOK(CConfig::LoadCharacter, CConfig::LoadCharacter_hook);
    ATTACH_HOOK(CConfig::LoadGlobal, CConfig::LoadGlobal_hook);
    ATTACH_HOOK(CConfig::SaveGlobal, CConfig::SaveGlobal_hook);
    ATTACH_HOOK(CConfig::ApplySysOpt, CConfig::ApplySysOpt_hook);
    ATTACH_HOOK(CUISysOpt::OnCreate, CUISysOpt::OnCreate_hook);
    ATTACH_HOOK(CUISysOpt::Destructor, CUISysOpt::Destructor_hook);
    // CUISysOpt::OnCreate - hide monster info combo box
    Patch4(0x00994525 + 1, 0);

    ATTACH_HOOK(CInputSystem::SetCursorVectorPos, CInputSystem::SetCursorVectorPos_hook);
    ATTACH_HOOK(CInputSystem::SetCursorPos, CInputSystem::SetCursorPos_hook);
    ATTACH_HOOK(CWndMan::GetCursorPos, CWndMan::GetCursorPos_hook);
    ATTACH_HOOK(CWndMan::GetHandlerFromPoint, CWndMan::GetHandlerFromPoint_hook);

    // CMapLoadable - handle view range
    ATTACH_HOOK(CMapLoadable::RestoreViewRange, CMapLoadable::RestoreViewRange_hook);
    PatchJmp(CMapLoadable__MakeGrid_jmp, &CMapLoadable__MakeGrid_hook);

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

    // CWnd::OnMoveWnd - handle snapping to screen bounds
    ATTACH_HOOK(CWnd__OnMoveWnd, CWnd__OnMoveWnd_hook);
    PatchJmp(0x009DFBFE, 0x009DFCC3);
    PatchJmp(0x009DFD01, 0x009DFDAA);
    PatchJmp(0x009DFDBB, 0x009DFE4D);
    PatchJmp(0x009DFE7E, 0x009DFF29);

    // CUIToolTip::MakeLayer - handle maximum bounds for CUIToolTip
    ATTACH_HOOK(CUIToolTip::MakeLayer, CUIToolTip::MakeLayer_hook);

    // CTemporaryStatView - reposition buff display
    ATTACH_HOOK(CTemporaryStatView::AdjustPosition, CTemporaryStatView::AdjustPosition_hook);
    ATTACH_HOOK(CTemporaryStatView::ShowToolTip, CTemporaryStatView::ShowToolTip_hook);
    ATTACH_HOOK(CTemporaryStatView::FindIcon, CTemporaryStatView::FindIcon_hook);

    // CAvatarMegaphone - reposition avatar megaphone
    PatchCall(0x0045B341, &CAvatarMegaphone__raw_RelMove_hook1, 6); // CAvatarMegaphone::HelloAvatarMegaphone - start position
    PatchCall(0x0045B421, &CAvatarMegaphone__raw_RelMove_hook2, 6); // CAvatarMegaphone::HelloAvatarMegaphone - end position
    PatchCall(0x0045B8A1, &CAvatarMegaphone__raw_RelMove_hook2, 6); // CAvatarMegaphone::ByeAvatarMegaphone - start position
    PatchCall(0x0045B985, &CAvatarMegaphone__raw_RelMove_hook1, 6); // CAvatarMegaphone::ByeAvatarMegaphone - end position

    // CSlideNotice - sliding notice position and width
    ATTACH_HOOK(CSlideNotice::SetMsg, CSlideNotice::SetMsg_hook);
    Patch4(0x007E15BE + 1, SCREEN_WIDTH_MAX); // CSlideNotice::CSlideNotice
    Patch4(0x007E16BE + 1, SCREEN_WIDTH_MAX); // CSlideNotice::OnCreate
    Patch4(0x007E1E07 + 2, SCREEN_WIDTH_MAX); // CSlideNotice::SetMsg
}