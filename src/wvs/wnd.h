#pragma once
#include "hook.h"
#include "wvs/gobj.h"
#include "wvs/msghandler.h"
#include "ztl/ztl.h"
#include <windows.h>

class CCtrlWnd;
struct DRAGCTX;

class CWnd : public IGObj, public IUIMsgHandler, public ZRefCounted {
public:
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

    MEMBER_AT(IWzGr2DLayerPtr, 0x18, m_pLayer)
    MEMBER_AT(IWzGr2DLayerPtr, 0x20, m_pOverlabLayer)
    MEMBER_AT(int, 0x24, m_width)
    MEMBER_AT(int, 0x28, m_height)
    MEMBER_AT(int, 0x3C, m_bScreenCoord)
    MEMBER_AT(POINT, 0x48, m_ptCursorRel)
    MEMBER_HOOK(void, 0x009DE4D2, CreateWnd, int l, int t, int w, int h, int z, int bScreenCoord, void* pData, int bSetFocus) // resolution.cpp

    virtual void Update() override {}
    virtual int OnDragDrop(int nState, DRAGCTX& ctx, int rx, int ry) { return 0; }
    virtual void PreCreateWnd(int l, int t, int w, int h, int z, int bScreenCoord, void* pData) {}
    virtual void OnCreate(void* pData) {}
    virtual void OnDestroy(void* pData) {}
    virtual void OnMoveWnd(int l, int t) {}
    virtual void OnEndMoveWnd() {}
    virtual void OnChildNotify(unsigned int nId, unsigned int param1, unsigned int param2) {}
    virtual void OnButtonClicked(unsigned int nId) {}
    virtual int HitTest(unsigned int rx, unsigned int ry, CCtrlWnd** ppCtrl) { return 0; }
    virtual int OnActivate(int bActive) { return 0; }
    virtual void Draw(const RECT* pRect) {}
    virtual int IsMyAddOn(CWnd* pWnd) { return 0; }

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
};


class CUIWnd : public CWnd {
public:
    MEMBER_AT(int, 0x588, m_nUIType)

    virtual void OnCreate(void* pData, void* sUOL, int bMultiBg) {}
};