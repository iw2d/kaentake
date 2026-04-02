#pragma once
#include "hook.h"
#include "wvs/gobj.h"
#include "wvs/msghandler.h"
#include "ztl/ztl.h"
#include <windows.h>


class CWnd;
struct DRAGCTX;

class CCtrlWnd : public IGObj, public IUIMsgHandler, public ZRefCounted {
public:
    unsigned char pad0[0x34 - sizeof(IGObj) - sizeof(IUIMsgHandler) - sizeof(ZRefCounted)];

    virtual void Update() override {}
    virtual int OnDragDrop(int nState, DRAGCTX& ctx, int rx, int ry) { return 0; }
    virtual void CreateCtrl(CWnd* pParent, unsigned int nId, int l, int t, int w, int h, void* pData) {}
    virtual void Destroy() {}
    virtual void OnCreate(void* pData) {}
    virtual void OnDestroy(void* pData) {}
    virtual int HitTest(unsigned int rx, unsigned int ry) { return 0; }
    virtual void Draw(int rx, int ry, const RECT* pRect) {}
};
static_assert(sizeof(CCtrlWnd) == 0x34);


class CCtrlButton : public CCtrlWnd {
public:
    struct CREATEPARAM {
        int bAcceptFocus;
        int bDrawBack;
        int bAnimateOnce;
        ZXString<wchar_t> sUOL;

        CREATEPARAM() : bAcceptFocus(0), bDrawBack(0), bAnimateOnce(0) {
        }
        ~CREATEPARAM() {
        }
    };
    static_assert(sizeof(CREATEPARAM) == 0x10);

    unsigned char pad0[0x5A4 - sizeof(CCtrlWnd)];

    virtual void CreateCtrl(CWnd* pParent, unsigned int nId, int l, int t, int decClickArea, void* pData) {}
    virtual ~CCtrlButton() {
        reinterpret_cast<void(__thiscall*)(CCtrlButton*)>(0x00425A3B)(reinterpret_cast<CCtrlButton*>(reinterpret_cast<uintptr_t>(this) + 0x8));
    }
    CCtrlButton() {
        reinterpret_cast<void(__thiscall*)(CCtrlButton*)>(0x004258E4)(this);
    }
};
static_assert(sizeof(CCtrlButton) == 0x5A4);


class CCtrlComboBox : public CCtrlWnd {
public:
    unsigned char pad0[0x10C - sizeof(CCtrlWnd)];
    MEMBER_AT(int, 0x68, m_nSelect)

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

    virtual void CreateCtrl(void* pParent, unsigned int nId, int nType, int l, int t, int w, int h, void* pData) {}
    virtual ~CCtrlComboBox() override {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*)>(0x004C4372)(reinterpret_cast<CCtrlComboBox*>(reinterpret_cast<uintptr_t>(this) + 0x8));
    }
    CCtrlComboBox() {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*)>(0x004C4259)(this);
    }

    void AddItem(const char* sItemName, unsigned int dwParam) {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*, const char*, unsigned int)>(0x004C6DD6)(this, sItemName, dwParam);
    }
    void SetSelect(int nSelect) {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*, int)>(0x004C738B)(this, nSelect);
    }
};
static_assert(sizeof(CCtrlComboBox) == 0x10C);