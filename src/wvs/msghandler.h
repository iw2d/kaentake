#pragma once
#include "wvs/rtti.h"


class IDraggable;

class IUIMsgHandler {
public:
    inline static CRTTI& ms_RTTI_IUIMsgHandler = *reinterpret_cast<CRTTI*>(0x00BEDBB8);

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


class CInPacket;

class INetMsgHandler {
public:
    virtual void OnPacket(int32_t nType, CInPacket& iPacket) {}
};