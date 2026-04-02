#pragma once
#include "hook.h"
#include "ztl/ztl.h"


class GW_ItemSlotEquip;

class CUIToolTip {
public:
    enum { // PrintValue Type
        PT_INC = 0x0,
        PT_VALUE = 0x1,
        PT_PERCENT = 0x2,
    };

    MEMBER_AT(int, 0x8, m_nHeight)
    MEMBER_AT(int, 0xC, m_nWidth)
    MEMBER_AT(IWzGr2DLayerPtr, 0x10, m_pLayer)

    // resolution.cpp
    MEMBER_HOOK(IWzCanvasPtr*, 0x008F3141, MakeLayer, IWzCanvasPtr* result, int nLeft, int nTop, int bDoubleOutline, int bLogin, int bCharToolTip, unsigned int uColor)

    // tooltip.cpp
    MEMBER_HOOK(void, 0x008ECA0C, SetToolTip_Equip_Basic, GW_ItemSlotEquip* pe)

    virtual ~CUIToolTip() {
        reinterpret_cast<void(__thiscall*)(CUIToolTip*)>(0x008E6BA3)(this);
    }
    CUIToolTip() {
        reinterpret_cast<void(__thiscall*)(CUIToolTip*)>(0x008E49B5)(this);
    }
    void ClearToolTip() {
        reinterpret_cast<void(__thiscall*)(CUIToolTip*)>(0x008E6E23)(this);
    }
    void AddInfoEx(int nType, int nSubType, ZXString<char> sContext, ZXString<char> sSubContext, int bUseDot, int nAlign) {
        reinterpret_cast<void(__thiscall*)(CUIToolTip*, int, int, ZXString<char>, ZXString<char>, int, int)>(0x008F39E1)(this, nType, nSubType, sContext, sSubContext, bUseDot, nAlign);
    }
    void PrintValue(int nType, int nValue, ZXString<char> sProperty, int bShowAlways) {
        reinterpret_cast<void(__thiscall*)(CUIToolTip*, int, int, ZXString<char>, int)>(0x008E7836)(this, nType, nValue, sProperty, bShowAlways);
    }
    void PrintValueEx(int nType, int nValue, int nBase, ZXString<char> sProperty, int bShowAlways) {
        if (!bShowAlways && nValue <= 0 && nBase <= 0) {
            return;
        }
        if (nValue == nBase || nType == PT_PERCENT) {
            PrintValue(nType, nValue, sProperty, bShowAlways);
            return;
        }

        ZXString<char> sDelta;
        if (nValue > nBase) {
            sDelta.Format("%d + %d", nBase, nValue - nBase);
        } else {
            sDelta.Format("%d - %d", nBase, nBase - nValue);
        }

        ZXString<char> sValue;
        switch (nType) {
        case PT_INC:
            if (nValue >= 0) {
                sValue.Format(" +%d (%s)", nValue, sDelta);
            } else {
                sValue.Format(" -%d (%s)", nValue, sDelta);
            }
            break;
        case PT_VALUE:
            sValue.Format(" %d (%s)", nValue, sDelta);
            break;
        }
        if (!sValue.IsEmpty()) {
            AddInfoEx(14, 16, sProperty, sValue, 1, 1001);
        }
    }
};