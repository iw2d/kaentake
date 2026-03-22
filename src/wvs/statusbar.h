#pragma once
#include "hook.h"
#include "wvs/wnd.h"
#include "ztl/ztl.h"


class CUIStatusBar : public CWnd, public TSingleton<CUIStatusBar, 0x00BEC208> {
public:
    MEMBER_AT(int, 0xD10, m_bQuickSlotUp)
    MEMBER_ARRAY_AT(IWzCanvasPtr, 0xD84, m_aCanvasSkillCooltime, 16)

    // resolution.cpp
    MEMBER_HOOK(int, 0x008DE8D5, GetShortCutIndexByPos, int x, int y)
};