#include "pch.h"
#include "hook.h"
#include "wvs/iteminfo.h"
#include "wvs/util.h"
#include "ztl/ztl.h"
#include <memory>


struct GW_ItemSlotBase : public ZRefCounted {
    virtual ~GW_ItemSlotBase() = 0;
    virtual int IsProtectedItem() = 0;
    virtual int IsPreventSlipItem() = 0;
    virtual int IsSupportWarmItem() = 0;
    virtual int IsBindedItem() = 0;
    virtual int IsPossibleTradingItem() = 0;
    virtual int GetType() = 0;
    // ... ignore unnecessary virtual methods
};

class CUserLocal : public TSingleton<CUserLocal, 0x00BEBF98> {
};

ZRef<GW_ItemSlotBase> GetActivePetItemSlot(int nIndex) {
    // CUser::GetActivePetItemSlot
    ZRef<GW_ItemSlotBase> pItemSlot;
    reinterpret_cast<ZRef<GW_ItemSlotBase>*(__thiscall*)(CUserLocal*, ZRef<GW_ItemSlotBase>*, int)>(0x0092EC12)(CUserLocal::GetInstance(), std::addressof(pItemSlot), nIndex);
    return pItemSlot;
}

void __fastcall CItemInfo__DrawItemIconForSlot_helper(CItemInfo* pThis, void* _EDX, GW_ItemSlotBase* pItem, IWzCanvasPtr pCanvas, int nItemID, int x, int y, int bProtectedItem, int bMag2, int bPetDead, int bHideCashIcon, int nEquipItemQuality, int bHideQualityIcon) {
    int nPetIndex = -1;
    if (pItem->GetType() == 3) {
        for (int i = 0; i < 3; ++i) {
            auto pPetItemSlot = GetActivePetItemSlot(i);
            if (pPetItemSlot == pItem) {
                nPetIndex = i;
                break;
            }
        }
        if (nPetIndex >= 0) {
            pCanvas->DrawRectangle(x + 1, y - 31, 31, 31, 0xFFBBCCDD);
        }
    }
    pThis->DrawItemIconForSlot(pCanvas, nItemID, x, y, bProtectedItem, bMag2, bPetDead, bHideCashIcon, nEquipItemQuality, bHideCashIcon);
    if (nPetIndex == 0) {
        IWzCanvasPtr pBossPetIcon = get_unknown(get_rm()->GetObjectA(L"UI/UIWindow.img/Item/bossPetIcon"));
        pCanvas->CopyEx(x - 1, y - 37, pBossPetIcon, CANVAS_ALPHATYPE::CA_REMOVEALPHA, 0, 0, 0, 0, 0, 0);
    }
}

static auto CItemInfo__DrawItemIconForSlot_jmp = 0x0081DEE9;
static auto CItemInfo__DrawItemIconForSlot_ret = 0x0081DEEE;
void __declspec(naked) CItemInfo__DrawItemIconForSlot_hook() {
    __asm {
        push    esi ; GW_ItemSlotBase*
        call    CItemInfo__DrawItemIconForSlot_helper
        jmp     [ CItemInfo__DrawItemIconForSlot_ret ]
    }
}


void AttachIconIconMod() {
    PatchJmp(CItemInfo__DrawItemIconForSlot_jmp, &CItemInfo__DrawItemIconForSlot_hook); // CUIItem::Draw
}
