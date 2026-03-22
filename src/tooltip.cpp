#include "pch.h"
#include "hook.h"
#include "constants.h"
#include "wvs/secure.h"
#include "wvs/iteminfo.h"
#include "ztl/ztl.h"


class GW_ItemSlotEquip {
public:
    MEMBER_AT(TSecType<int>, 0xC, nItemID)
    MEMBER_AT(ZtlSecurePacked<unsigned char>, 0x28, nRUC)
    MEMBER_AT(ZtlSecure<short>, 0x34, niSTR)
    MEMBER_AT(ZtlSecure<short>, 0x3C, niDEX)
    MEMBER_AT(ZtlSecure<short>, 0x44, niINT)
    MEMBER_AT(ZtlSecure<short>, 0x4C, niLUK)
    MEMBER_AT(ZtlSecure<short>, 0x54, niMaxHP)
    MEMBER_AT(ZtlSecure<short>, 0x5C, niMaxMP)
    MEMBER_AT(ZtlSecure<short>, 0x64, niPAD)
    MEMBER_AT(ZtlSecure<short>, 0x6C, niMAD)
    MEMBER_AT(ZtlSecure<short>, 0x74, niPDD)
    MEMBER_AT(ZtlSecure<short>, 0x7C, niMDD)
    MEMBER_AT(ZtlSecure<short>, 0x84, niACC)
    MEMBER_AT(ZtlSecure<short>, 0x8C, niEVA)
    MEMBER_AT(ZtlSecure<short>, 0x94, niCraft)
    MEMBER_AT(ZtlSecure<short>, 0x9C, niSpeed)
    MEMBER_AT(ZtlSecure<short>, 0xA4, niJump)
    MEMBER_AT(ZtlSecure<short>, 0xAC, nAttribute)
};

class CUIToolTip {
public:
    enum { // PrintValue Type
        PT_INC = 0x0,
        PT_VALUE = 0x1,
        PT_PERCENT = 0x2,
    };

    MEMBER_HOOK(void, 0x008ECA0C, SetToolTip_Equip_Basic, GW_ItemSlotEquip* pe)

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

void CUIToolTip::SetToolTip_Equip_Basic_hook(GW_ItemSlotEquip* pe) {
    int nItemID = pe->nItemID;
    auto pEquipItem = CItemInfo::GetInstance()->GetEquipItem(nItemID);
    if (!pEquipItem) {
        return;
    }
    // get_weapon_category_name
    ZXString<char> sWeaponCategory;
    reinterpret_cast<ZXString<char>*(__cdecl*)(ZXString<char>*, int)>(0x005C99FC)(&sWeaponCategory, nItemID);
    if (!sWeaponCategory.IsEmpty()) {
        AddInfoEx(14, 15, "CATEGORY :", sWeaponCategory, 1, 1001);
    }
    // get_item_category_name
    ZXString<char> sItemCategory;
    reinterpret_cast<ZXString<char>*(__cdecl*)(ZXString<char>*, int)>(0x005C9E61)(&sItemCategory, nItemID);
    if (!sItemCategory.IsEmpty()) {
        AddInfoEx(14, 15, "CATEGORY :", sItemCategory, 1, 1001);
    }
    // get_weapon_attack_speed
    ZXString<char> sAttackSpeed;
    reinterpret_cast<ZXString<char>*(__cdecl*)(ZXString<char>*, int)>(0x005C9AFA)(&sAttackSpeed, nItemID);
    if (!sAttackSpeed.IsEmpty()) {
        AddInfoEx(14, 15, "ATTACK SPEED :", sAttackSpeed, 1, 1001);
    }

    PrintValueEx(PT_INC, pe->niSTR, pEquipItem->niSTR, "STR :", 0);
    PrintValueEx(PT_INC, pe->niDEX, pEquipItem->niDEX, "DEX :", 0);
    PrintValueEx(PT_INC, pe->niINT, pEquipItem->niINT, "INT :", 0);
    PrintValueEx(PT_INC, pe->niLUK, pEquipItem->niLUK, "LUK :", 0);
    PrintValueEx(PT_INC, pe->niMaxHP, pEquipItem->niMaxHP, "HP :", 0);
    PrintValueEx(PT_INC, pe->niMaxMP, pEquipItem->niMaxMP, "MP :", 0);

    PrintValueEx(PT_VALUE, pe->niPAD, pEquipItem->niPAD, "WEAPON ATTACK :", 0);
    PrintValueEx(PT_VALUE, pe->niMAD, pEquipItem->niMAD, "MAGIC ATTCK :", 0);
    PrintValueEx(PT_VALUE, pe->niPDD, pEquipItem->niPDD, "WEAPON DEF. :", 0);
    PrintValueEx(PT_VALUE, pe->niMDD, pEquipItem->niMDD, "MAGIC DEF. :", 0);

    PrintValueEx(PT_INC, pe->niACC, pEquipItem->niACC, "ACCURACY :", 0);
    PrintValueEx(PT_INC, pe->niEVA, pEquipItem->niEVA, "AVOIDABILITY :", 0);
    PrintValueEx(PT_INC, pe->niCraft, pEquipItem->niCraft, "HANDS :", 0);
    PrintValueEx(PT_INC, pe->niSpeed, pEquipItem->niSpeed, "SPEED :", 0);
    PrintValueEx(PT_INC, pe->niJump, pEquipItem->niJump, "JUMP :", 0);

    PrintValue(PT_PERCENT, pEquipItem->nKnockback, "THE RATE OF KNOCK-BACK :", 0);
    if (pe->nAttribute & 2) {
        AddInfoEx(14, 15, "ADD PREVENT SLIPPING", "", 1, 1001);
    }
    if (pe->nAttribute & 4) {
        AddInfoEx(14, 15, "ADD PREVENT COLDNESS", "", 1, 1001);
    }
    if (pEquipItem->nRUC) {
        PrintValue(PT_VALUE, pe->nRUC, "NUMBER OF UPGRADES AVAILABLE :", 1);
    }
}


void AttachToolTipMod() {
    if (!CONSTANTS_TOOLTIP_DELTA) {
        return;
    }

    ATTACH_HOOK(CUIToolTip::SetToolTip_Equip_Basic, CUIToolTip::SetToolTip_Equip_Basic_hook);
}