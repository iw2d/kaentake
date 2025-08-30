#pragma once
#include "hook.h"
#include "wvs/gobj.h"
#include "wvs/msghandler.h"
#include "ztl/zalloc.h"
#include <windows.h>


class CWnd;
struct DRAGCTX;

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
    void SetSelect(int nSelect) {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*, int)>(0x004C738B)(this, nSelect);
    }
};
