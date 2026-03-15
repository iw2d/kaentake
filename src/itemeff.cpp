#include "pch.h"
#include "hook.h"
#include "wvs/avatar.h"
#include "wvs/iteminfo.h"
#include "wvs/util.h"
#include "ztl/ztl.h"
#include <map>

#define AVATAR_OFFSET 0x88


class CUser {
public:
    MEMBER_AT(CAvatar, AVATAR_OFFSET, m_CAvatar)

    inline int LoadLayer(Ztl_bstr_t bsUOL, int bLeft, USERLAYER& l, int* pnRepeatStartIndex) {
        return reinterpret_cast<int(__thiscall*)(CUser*, Ztl_bstr_t, int, USERLAYER&, int*)>(0x00941417)(this, bsUOL, bLeft, l, pnRepeatStartIndex);
    }
};

static std::map<int, IWzPropertyPtr> g_mPropItemEffect;

static auto CItemInfo__IterateItemInfo = reinterpret_cast<int(__thiscall*)(CItemInfo*)>(0x005CA71C);
int __fastcall CItemInfo__IterateItemInfo_hook(CItemInfo* pThis, void* _EDX) {
    IWzPropertyPtr pItemEff = get_rm()->GetObjectA(L"Effect/ItemEff.img").GetUnknown();
    IEnumVARIANTPtr pEnum = pItemEff->_NewEnum;
    while (true) {
        Ztl_variant_t vNext;
        ULONG uCeltFetched;
        if (FAILED(pEnum->Next(1, &vNext, &uCeltFetched)) || uCeltFetched == 0) {
            break;
        }
        Ztl_bstr_t sNext = V_BSTR(&vNext);
        int nItemID = wcstol(sNext.GetBSTR(), nullptr, 10);
        IWzPropertyPtr pProp = pItemEff->item[sNext].GetUnknown();
        IWzPropertyPtr pEffect;
        IUnknownPtr pUnknown = pProp->item[L"effect"].GetUnknown();
        if (SUCCEEDED(pUnknown.QueryInterface(__uuidof(IWzPropertyPtr), &pEffect))) {
            DEBUG_MESSAGE("Loaded Effect/ItemEff.img/%d", nItemID);
            g_mPropItemEffect[nItemID] = pEffect;
        }
    }
    return CItemInfo__IterateItemInfo(pThis);
}


void UpdateItemEff(CUser* pUser) {
    CAvatar* pAvatar = &pUser->m_CAvatar;
    for (auto i = 0; i < 60; ++i) {
        int nItemID = pAvatar->m_avatarLook.anHairEquip[i];
        auto pItemEffectLayer = &pAvatar->m_pCustomData->aItemEffectLayer[i];
        if (auto search = g_mPropItemEffect.find(nItemID); search != g_mPropItemEffect.end()) {
            int bFlip = pAvatar->m_pLayerUnderFace->flip;
            int nAction = pAvatar->GetCurrentAction(nullptr);
            if (pItemEffectLayer->nItemID == nItemID && pItemEffectLayer->nAction == nAction &&
                (!pItemEffectLayer->l.bFixed && pItemEffectLayer->bFlip == bFlip)) {
                continue;
            }
            pItemEffectLayer->nItemID = nItemID;
            pItemEffectLayer->nAction = nAction;
            pItemEffectLayer->bFlip = bFlip;

            // resolve UOL
            Ztl_bstr_t sActionName;
            // get_action_name_from_code(&sActionName, nAction);
            reinterpret_cast<Ztl_bstr_t*(__cdecl*)(Ztl_bstr_t*, int)>(0x004A8CE6)(&sActionName, nAction);
            IWzPropertyPtr pEffect = search->second;
            Ztl_variant_t vAction = pEffect->item[sActionName];
            wchar_t sUOL[1024];
            swprintf(sUOL, 1024, L"Effect/ItemEff.img/%d/effect/%ls", nItemID, vAction.vt == VT_EMPTY ? L"default" : sActionName.GetBSTR());

            // load layer and animate
            if (pUser->LoadLayer(sUOL, bFlip, pItemEffectLayer->l, nullptr)) {
                pItemEffectLayer->l.pLayer->Animate(GA_REPEAT);
            }
        } else {
            pItemEffectLayer->Reset();
        }
    }
}

static auto CUser__UpdateAdditionalLayer = reinterpret_cast<void(__thiscall*)(CUser*)>(0x00940EB7);

void __fastcall CUser__UpdateAdditionalLayer_hook(CUser* pThis, void* _EDX) {
    CUser__UpdateAdditionalLayer(pThis);
    UpdateItemEff(pThis);
}

static auto CUser__OnAvatarModified = reinterpret_cast<void(__thiscall*)(void*)>(0x0092E916);

void __fastcall CUser__OnAvatarModified_hook(void* _ECX, void* _EDX) {
    CUser__OnAvatarModified(_ECX);
    UpdateItemEff(reinterpret_cast<CUser*>(reinterpret_cast<uintptr_t>(_ECX) - AVATAR_OFFSET));
}

static auto CUser__SetMoveAction = reinterpret_cast<void(__thiscall*)(void*, int, int)>(0x0092ECD1);

void __fastcall CUser__SetMoveAction_hook(void* _ECX, void* _EDX, int nMA, int bReload) {
    CUser__SetMoveAction(_ECX, nMA, bReload);
    UpdateItemEff(reinterpret_cast<CUser*>(reinterpret_cast<uintptr_t>(_ECX) - AVATAR_OFFSET));
}


void AttachItemEffectMod() {
    ATTACH_HOOK(CItemInfo__IterateItemInfo, CItemInfo__IterateItemInfo_hook);
    ATTACH_HOOK(CUser__UpdateAdditionalLayer, CUser__UpdateAdditionalLayer_hook);
    ATTACH_HOOK(CUser__OnAvatarModified, CUser__OnAvatarModified_hook);
    ATTACH_HOOK(CUser__SetMoveAction, CUser__SetMoveAction_hook);
}