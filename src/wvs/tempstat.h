#pragma once
#include "hook.h"
#include "wvs/tooltip.h"
#include "ztl/ztl.h"


class CTemporaryStatView {
public:
    struct TEMPORARY_STAT : public ZRefCounted {
        unsigned char pad0[0x40 - sizeof(ZRefCounted)];
        MEMBER_AT(int, 0x1C, nType)
        MEMBER_AT(int, 0x20, nID)
        MEMBER_AT(IWzGr2DLayerPtr, 0x28, pLayer)
        MEMBER_AT(IWzGr2DLayerPtr, 0x2C, pLayerShadow)
        MEMBER_AT(int, 0x30, nIndexShadow)
        MEMBER_AT(int, 0x34, bNoShadow)
        MEMBER_AT(int, 0x38, tLeft)
        MEMBER_AT(int, 0x3C, tLeftUnit)
    };
    MEMBER_AT(ZList<ZRef<TEMPORARY_STAT>>, 0x4, m_lTemporaryStat)

    // resolution.cpp
    MEMBER_HOOK(void, 0x007B2BB0, AdjustPosition)
    MEMBER_HOOK(int, 0x007B2E58, ShowToolTip, CUIToolTip& uiToolTip, const POINT& ptCursor, int rx, int ry)
    MEMBER_HOOK(int, 0x007B3055, FindIcon, const POINT& ptCursor, int& nType, int& nID)

    // tempstat.cpp
    MEMBER_HOOK(void, 0x007B2829, Update)
};
