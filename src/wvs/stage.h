#pragma once
#include "wvs/gobj.h"
#include "wvs/msghandler.h"
#include "ztl/zalloc.h"


class CStage : public IGObj, public IUIMsgHandler, public INetMsgHandler, public ZRefCounted {
public:
    virtual void Init(void* pParam) {}
    virtual void Close() {}
};
