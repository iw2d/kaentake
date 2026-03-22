#pragma once
#include "hook.h"
#include "wvs/wnd.h"
#include "ztl/ztl.h"


class CUIStatusBar : public CWnd, public TSingleton<CUIStatusBar, 0x00BEC208> {
public:
    MEMBER_AT(int, 0xD10, m_bQuickSlotUp)
};