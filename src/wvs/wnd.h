#pragma once
#include "hook.h"
#include "wvs/gobj.h"
#include "wvs/msghandler.h"
#include "ztl/zalloc.h"
#include "ztl/zcom.h"
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
    void CreateWnd(int l, int t, int w, int h, int z, int bScreenCoord, void* pData, int bSetFocus) {
        reinterpret_cast<void(__thiscall*)(CWnd*, int, int, int, int, int, int, void*, int)>(0x009DE4D2)(this, l, t, w, h, z, bScreenCoord, pData, bSetFocus);
    }
    void Destroy() {
        reinterpret_cast<void(__thiscall*)(CWnd*)>(0x009E00AF)(this);
    }
    void InvalidateRect(const RECT* pRect) {
        reinterpret_cast<void(__thiscall*)(CWnd*, const RECT*)>(0x009E04C9)(this, pRect);
    }
};