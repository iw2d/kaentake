#include "pch.h"
#include "hook.h"
#include "wvs/secure.h"
#include "wvs/tooltip.h"
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
    ATTACH_HOOK(CUIToolTip::SetToolTip_Equip_Basic, CUIToolTip::SetToolTip_Equip_Basic_hook);
}