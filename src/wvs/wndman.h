#pragma once
#include "hook.h"
#include "ztl/zcom.h"
#include "ztl/tsingleton.h"


class CWndMan : public TSingleton<CWndMan, 0x00BEC20C> {
public:
    MEMBER_AT(IWzVector2DPtr, 0xDC, m_pOrgWindow)
};