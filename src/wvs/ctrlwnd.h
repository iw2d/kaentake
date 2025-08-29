#pragma once
#include "ztl/zalloc.h"

struct DRAGCTX;
struct RECT;
class CWnd;
class IDraggable;

class CRTTI {
public:
    const CRTTI* m_pPrev;

    CRTTI(const CRTTI* pPrev) : m_pPrev(pPrev) {
    }
    int IsKindOf(const CRTTI* pRTTI) const {
        CRTTI result = {this};
        while (result.m_pPrev != pRTTI) {
            result.m_pPrev = result.m_pPrev->m_pPrev;
            if (!result.m_pPrev) {
                return 0;
            }
        }
        return 1;
    }
};


class IGObj {
public:
    virtual void Update() {}
};


class IUIMsgHandler {
public:
    inline static CRTTI& ms_RTTI_IUIMsgHandler = *reinterpret_cast<CRTTI*>(0x00C6B348);

    virtual void OnKey(unsigned int wParam, unsigned int lParam) {}
    virtual int OnSetFocus(int bFocus) { return 0; }
    virtual void OnMouseButton(unsigned int msg, unsigned int wParam, int rx, int ry) {}
    virtual int OnMouseMove(int rx, int ry) { return 0; }
    virtual int OnMouseWheel(int rx, int ry, int nWheel) { return 0; }
    virtual void OnMouseEnter(int bEnter) {}
    virtual void OnDraggableMove(int nState, IDraggable* pObj, int rx, int ry) {}
    virtual void SetEnable(int bEnable) {}
    virtual int IsEnabled() const { return 1; }
    virtual void SetShow(int bShow) {}
    virtual int IsShown() const { return 1; }
    virtual int GetAbsLeft() { return 0; }
    virtual int GetAbsTop() { return 0; }
    virtual void ClearToolTip() {}
    virtual void OnIMEModeChange(char cMode) {}
    virtual void OnIMEResult(const char* sComp) {}
    virtual void OnIMEComp(const char* sComp, void* adwCls, unsigned int nClsIdx, int nCursor, void* lCand, int nBegin, int nPage, int nCur) {}
    virtual const CRTTI* GetRTTI() const {
        return &ms_RTTI_IUIMsgHandler;
    }
    virtual int IsKindOf(const CRTTI* pRTTI) const {
        return ms_RTTI_IUIMsgHandler.IsKindOf(pRTTI);
    }
};


class CCtrlWnd : public IGObj, public IUIMsgHandler, public ZRefCounted {
public:
    virtual void Update() override {}
    virtual int OnDragDrop(int nState, DRAGCTX& ctx, int rx, int ry) { return 0; }
    virtual void CreateCtrl(CWnd* pParent, unsigned int nId, int l, int t, int w, int h, void* pData) {}
    virtual void Destroy() {}
    virtual void OnCreate(void* pData) {}
    virtual void OnDestroy(void* pData) {}
    virtual int HitTest(unsigned int rx, unsigned int ry) { return 0; }
    virtual void Draw(int rx, int ry, const RECT* pRect) {}
};


class CCtrlComboBox : public CCtrlWnd {
public:
    unsigned char pad0[0x10C - sizeof(CCtrlWnd)];

    struct CREATEPARAM {
        unsigned char pad0[0x50];
        MEMBER_AT(int, 0x0, nBackColor)
        MEMBER_AT(int, 0x4, nBackFocusedColor)
        MEMBER_AT(int, 0x8, nBorderColor)

        CREATEPARAM() {
            reinterpret_cast<void(__thiscall*)(CREATEPARAM*)>(0x004B620C)(this);
        }
        ~CREATEPARAM() {
            reinterpret_cast<void(__thiscall*)(CREATEPARAM*)>(0x004B63AE)(this);
        }
    };

    CCtrlComboBox() {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*)>(0x004C4259)(this);
    }
    virtual ~CCtrlComboBox() override {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*)>(0x004C4372)(this);
    }
    virtual void CreateCtrl(void* pParent, unsigned int nId, int nType, int l, int t, int w, int h, void* pData) {}

    void AddItem(const char* sItemName, unsigned int dwParam) {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*, const char*, unsigned int)>(0x004C6DD6)(this, sItemName, dwParam);
    }
};
