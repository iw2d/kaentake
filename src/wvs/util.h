#pragma once
#include "ztl/zcom.h"


IWzGr2DPtr& get_gr() {
    return *reinterpret_cast<IWzGr2DPtr*>(0x00BF14EC);
}

// implementation in resolution.cpp
int get_screen_width();
int get_screen_height();
int get_adjust_cy();