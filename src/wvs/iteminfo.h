#pragma once
#include "hook.h"
#include "wvs/secure.h"
#include "ztl/ztl.h"
#include <memory>


class CItemInfo : public TSingleton<CItemInfo, 0x00BE78D8> {
public:
    struct EQUIPITEM {
        MEMBER_AT(ZtlSecure<unsigned char>, 0x9C, nRUC)
        MEMBER_AT(ZtlSecure<short>, 0xAC, niSTR)
        MEMBER_AT(ZtlSecure<short>, 0xB4, niDEX)
        MEMBER_AT(ZtlSecure<short>, 0xBC, niINT)
        MEMBER_AT(ZtlSecure<short>, 0xC4, niLUK)
        MEMBER_AT(ZtlSecure<short>, 0xCC, niMaxHP)
        MEMBER_AT(ZtlSecure<short>, 0xD4, niMaxMP)
        MEMBER_AT(ZtlSecure<short>, 0xDC, niPAD)
        MEMBER_AT(ZtlSecure<short>, 0xE4, niMAD)
        MEMBER_AT(ZtlSecure<short>, 0xEC, niPDD)
        MEMBER_AT(ZtlSecure<short>, 0xF4, niMDD)
        MEMBER_AT(ZtlSecure<short>, 0xFC, niACC)
        MEMBER_AT(ZtlSecure<short>, 0x104, niEVA)
        MEMBER_AT(ZtlSecure<short>, 0x10C, niCraft)
        MEMBER_AT(ZtlSecure<short>, 0x114, niSpeed)
        MEMBER_AT(ZtlSecure<short>, 0x11C, niJump)
        MEMBER_AT(ZtlSecure<int>, 0x134, nKnockback)
    };

    inline IWzPropertyPtr GetItemInfo(int nItemID) {
        IWzPropertyPtr result;
        reinterpret_cast<IWzPropertyPtr*(__thiscall*)(CItemInfo*, IWzPropertyPtr*, int)>(0x005DA83C)(this, std::addressof(result), nItemID);
        return result;
    }

    inline EQUIPITEM* GetEquipItem(int nItemID) {
        return reinterpret_cast<EQUIPITEM*(__thiscall*)(CItemInfo*, int)>(0x005CA785)(this, nItemID);
    }
};
