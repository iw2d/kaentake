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
    MEMBER_AT(IWzGr2DLayerPtr, 0x18, m_pLayer)
    MEMBER_AT(IWzGr2DLayerPtr, 0x20, m_pOverlabLayer)

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
    virtual int IsMyAddOn() { return 0; }

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