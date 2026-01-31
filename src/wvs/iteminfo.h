#pragma once
#include "hook.h"
#include "ztl/ztl.h"


class CItemInfo : public TSingleton<CItemInfo, 0x00BE78D8> {
public:
    IWzPropertyPtr GetItemInfo(int32_t nItemID) {
        IWzPropertyPtr result;
        reinterpret_cast<IWzPropertyPtr*(__thiscall*)(CItemInfo*, IWzPropertyPtr*, int)>(0x005DA83C)(this, std::addressof(result), nItemID);
        return result;
    }
};
