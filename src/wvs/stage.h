#pragma once
#include "wvs/gobj.h"
#include "wvs/msghandler.h"
#include "ztl/ztl.h"


class CStage : public IGObj, public IUIMsgHandler, public INetMsgHandler, public ZRefCounted {
public:
    virtual void Init(void* pParam) {}
    virtual void Close() {}
};

inline ZRef<CStage>& get_stage() {
    return *reinterpret_cast<ZRef<CStage>*>(0x00BEDED0);
}