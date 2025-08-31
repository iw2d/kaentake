#pragma once
#include "ztl/zcom.h"


IWzGr2DPtr& get_gr() {
    return *reinterpret_cast<IWzGr2DPtr*>(0x00BF14EC);
}

int get_int32(Ztl_variant_t& v, int nDefault) {
    Ztl_variant_t vInt;
    if (V_VT(&v) == VT_EMPTY || V_VT(&v) == VT_ERROR || FAILED(ZComAPI::ZComVariantChangeType(&vInt, &v, 0, VT_I4))) {
        return nDefault;
    } else {
        return V_I4(&vInt);
    }
}

// implementation in resolution.cpp
int get_screen_width();
int get_screen_height();
int get_adjust_cy();