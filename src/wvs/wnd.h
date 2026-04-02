#pragma once
#include "hook.h"
#include "wvs/gobj.h"
#include "wvs/msghandler.h"
#include "wvs/tooltip.h"
#include "wvs/ctrlwnd.h"
#include "wvs/rtti.h"
#include "ztl/ztl.h"
#include <windows.h>


struct DRAGCTX;

class CWnd : public IGObj, public IUIMsgHandler, public ZRefCounted {
public:
    inline static CRTTI& ms_RTTI_CWnd = *reinterpret_cast<CRTTI*>(0x00C6194C);

    enum UIOrigin {
        Origin_LT = 0x0,
        Origin_CT = 0x1,
        Origin_RT = 0x2,
        Origin_LC = 0x3,
        Origin_CC = 0x4,
        Origin_RC = 0x5,
        Origin_LB = 0x6,
        Origin_CB = 0x7,
        Origin_RB = 0x8,
        Origin_NUM = 0x9,
    };

    unsigned char pad[0x6C - sizeof(IGObj) - sizeof(IUIMsgHandler) - sizeof(ZRefCounted)];
    MEMBER_AT(IWzGr2DLayerPtr, 0x18, m_pLayer)
    MEMBER_AT(IWzGr2DLayerPtr, 0x20, m_pOverlabLayer)
    MEMBER_AT(int, 0x24, m_width)
    MEMBER_AT(int, 0x28, m_height)
    MEMBER_AT(int, 0x3C, m_bScreenCoord)
    MEMBER_AT(POINT, 0x48, m_ptCursorRel)

    // resolution.cpp
    MEMBER_HOOK(void, 0x009DE4D2, CreateWnd, int l, int t, int w, int h, int z, int bScreenCoord, void* pData, int bSetFocus)

    virtual int OnDragDrop(int nState, DRAGCTX& ctx, int rx, int ry) {
        return 0;
    }
    virtual void PreCreateWnd(int l, int t, int w, int h, int z, int bScreenCoord, void* pData) {
        reinterpret_cast<void(__thiscall*)(CWnd*, int, int, int, int, int, int, void*)>(0x009DE7FB)(this, l, t, w, h, z, bScreenCoord, pData);
    }
    virtual void OnCreate(void* pData) {
        ;
    }
    virtual void OnDestroy() {
        ;
    }
    virtual void OnMoveWnd(int l, int t) {
        reinterpret_cast<void(__thiscall*)(CWnd*, int, int)>(0x009DEB57)(this, l, t);
    }
    virtual void OnEndMoveWnd() {
        reinterpret_cast<void(__thiscall*)(CWnd*)>(0x009E00A6)(this);
    }
    virtual void OnChildNotify(unsigned int nId, unsigned int param1, unsigned int param2) {
        if (param1 == 100) {
            OnButtonClicked(nId);
        }
    }
    virtual void OnButtonClicked(unsigned int nId) {
        ;
    }
    virtual int HitTest(int rx, int ry, CCtrlWnd** ppCtrl) {
        return reinterpret_cast<int(__thiscall*)(CWnd*, int, int, CCtrlWnd**)>(0x009E01E7)(this, rx, ry, ppCtrl);
    }
    virtual int OnActivate(int bActive) {
        return reinterpret_cast<int(__thiscall*)(CWnd*, int)>(0x009E03A6)(this, bActive);
    }
    virtual void Draw(const RECT* pRect) {
        reinterpret_cast<void(__thiscall*)(CWnd*, const RECT*)>(0x009E0502)(this, pRect);
    }
    virtual int IsMyAddOn(CWnd* pWnd) {
        return 0;
    }

    virtual int OnSetFocus(int bFocus) override {
        return reinterpret_cast<int(__thiscall*)(IUIMsgHandler*, int)>(0x009E0369)(this, bFocus);
    }
    virtual int OnMouseWheel(int rx, int ry, int nWheel) override {
        return reinterpret_cast<int(__thiscall*)(IUIMsgHandler*, int, int, int)>(0x009E02AE)(this, rx, ry, nWheel);
    }
    virtual void OnMouseEnter(int bEnter) override {
        reinterpret_cast<void(__thiscall*)(IUIMsgHandler*, int)>(0x009E01C8)(this, bEnter);
    }
    virtual int GetAbsLeft() override {
        return reinterpret_cast<int(__thiscall*)(IUIMsgHandler*)>(0x009E03C5)(this);
    }
    virtual int GetAbsTop() override {
        return reinterpret_cast<int(__thiscall*)(IUIMsgHandler*)>(0x009E0447)(this);
    }
    virtual const CRTTI* GetRTTI() const override {
        return &ms_RTTI_CWnd;
    }
    virtual int IsKindOf(const CRTTI* pRTTI) const override {
        return ms_RTTI_CWnd.IsKindOf(pRTTI);
    }

    virtual ~CWnd() override {
        // Due to MSVC ABI differences, the CWnd destructor expects `this` to be aligned to the ZRefCounted vftable
        reinterpret_cast<void(__thiscall*)(CWnd*)>(0x009DE438)(reinterpret_cast<CWnd*>(reinterpret_cast<uintptr_t>(this) + 0x8));
    }
    CWnd() {
        reinterpret_cast<void(__thiscall*)(CWnd*)>(0x009DE383)(this);
    }
    explicit CWnd(int nDummy) {
        ;
    }

    IWzGr2DLayerPtr GetLayer() {
        return m_pLayer;
    }
    IWzCanvasPtr GetCanvas() {
        if (m_pOverlabLayer) {
            return m_pOverlabLayer->canvas[0];
        } else {
            return m_pLayer->canvas[0];
        }
    }
    void MoveWnd(int l, int t) {
        m_pLayer->RelMove(l, t);
    }
    void InvalidateRect(const RECT* pRect) {
        reinterpret_cast<void(__thiscall*)(CWnd*, const RECT*)>(0x009E04C9)(this, pRect);
    }
    void Destroy() {
        reinterpret_cast<void(__thiscall*)(CWnd*)>(0x009E00AF)(this);
    }
};
static_assert(sizeof(CWnd) == 0x6C);


class CUIWnd : public CWnd {
public:
    inline static CRTTI& ms_RTTI_CUIWnd = *reinterpret_cast<CRTTI*>(0x00BF11DC);

    unsigned char pad0[0x5B0 - sizeof(CWnd)];
    MEMBER_AT(ZRef<CCtrlButton>, 0x6C, m_pBtClose)
    MEMBER_AT(CUIToolTip, 0x74, m_uiToolTip)
    MEMBER_AT(int, 0x588, m_nUIType)
    MEMBER_AT(int, 0x5A4, m_nOption)
    MEMBER_AT(ZArray<unsigned char>, 0x5A8, m_abOption)
    MEMBER_AT(ZXString<wchar_t>, 0x5AC, m_sBackgrndUOL)

    virtual void OnDestroy() override {
        reinterpret_cast<void(__thiscall*)(CUIWnd*)>(0x0092C544)(this);
    }
    virtual void OnButtonClicked(unsigned int nId) override {
        reinterpret_cast<void(__thiscall*)(CUIWnd*, unsigned int)>(0x0092C5AE)(this, nId);
    }
    virtual int HitTest(int rx, int ry, CCtrlWnd** ppCtrl) override {
        return reinterpret_cast<int(__thiscall*)(CUIWnd*, int, int, CCtrlWnd**)>(0x0092C5E4)(this, rx, ry, ppCtrl);
    }
    virtual void OnCreate(void* pData, ZXString<wchar_t> sUOL, int bMultiBg) {
        reinterpret_cast<void(__thiscall*)(CUIWnd*, void*, ZXString<wchar_t>, int)>(0x0092C2E8)(this, pData, sUOL, bMultiBg);
    }

    virtual void OnKey(unsigned int wParam, unsigned int lParam) override {
        reinterpret_cast<void(__thiscall*)(IUIMsgHandler*, unsigned int, unsigned int)>(0x0092C6EE)(this, wParam, lParam);
    }
    virtual int OnSetFocus(int bFocus) override {
        return 0;
    }
    virtual void OnMouseEnter(int bEnter) override {
        CWnd::OnMouseEnter(bEnter);
        ClearToolTip();
    }
    virtual void ClearToolTip() override {
        m_uiToolTip.ClearToolTip();
    }
    virtual const CRTTI* GetRTTI() const override {
        return &ms_RTTI_CUIWnd;
    }
    virtual int IsKindOf(const CRTTI* pRTTI) const override {
        return ms_RTTI_CUIWnd.IsKindOf(pRTTI);
    }

    virtual ~CUIWnd() override {
        // implement CUIWnd destructor so we dont call CWnd destructor twice
        destruct(&m_sBackgrndUOL);
        destruct(&m_abOption);
        destruct(&m_uiToolTip);
        destruct(&m_pBtClose);
    }
    CUIWnd(int nUIType, int nCloseType, int nCloseX, int nCloseY, int bBackgrnd, int nBackgrndX, int nBackgrndY) : CWnd(0) {
        reinterpret_cast<void(__thiscall*)(CUIWnd*, int, int, int, int, int, int, int)>(0x0092C0BF)(this, nUIType, nCloseType, nCloseX, nCloseY, bBackgrnd, nBackgrndX, nBackgrndY);
    }
};
static_assert(sizeof(CUIWnd) == 0x5B0);