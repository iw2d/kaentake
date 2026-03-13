#include "pch.h"
#include "hook.h"
#include "wvs/avatar.h"
#include "wvs/iteminfo.h"
#include "wvs/util.h"
#include "ztl/ztl.h"
#include <map>

#define AVATAR_OFFSET 0x88
#define AL_ITEMEFFECT 2 // share with AL_ADMINEFFECT, AL_ACTIVEITEMEFFECT

struct USERLAYER {
    enum POSTYPE {
        POS_BODY_ORIGIN = 0x0,
        POS_FACE_ORIGIN = 0x1,
        POS_CENTER = 0x2,
        POS_GROUND_ORIGIN = 0x3,
    };

    int bFixed;
    POSTYPE nPos; // USERLAYER::POSTYPE
    IWzGr2DLayerPtr pLayer;
};
static_assert(sizeof(USERLAYER) == 0xC);


class CUser {
public:
    struct ADDITIONALLAYER {
        int nDataForRepeat;
        int nType;
        int nData;
        USERLAYER l;
        USERLAYER l2;
    };
    static_assert(sizeof(ADDITIONALLAYER) == 0x24);

    MEMBER_AT(CAvatar, AVATAR_OFFSET, m_CAvatar)

    inline ADDITIONALLAYER* GetAdditionalLayer(int nType) {
        return reinterpret_cast<ADDITIONALLAYER*(__thiscall*)(CUser*, int)>(0x00940BC1)(this, nType);
    }
    inline ADDITIONALLAYER* RemoveAdditionalLayer(int nType) {
        return reinterpret_cast<ADDITIONALLAYER*(__thiscall*)(CUser*, int)>(0x00940CB1)(this, nType);
    }
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
    bool bItemEff = false;
    auto pAdditionalLayer = pUser->GetAdditionalLayer(AL_ITEMEFFECT);
    for (int nItemID : pAvatar->m_avatarLook.anHairEquip) {
        if (auto search = g_mPropItemEffect.find(nItemID); search != g_mPropItemEffect.end()) {
            bItemEff = true;

            int nAction = pAvatar->GetCurrentAction(nullptr);
            Ztl_bstr_t sActionName;
            // get_action_name_from_code(&sActionName, nAction);
            reinterpret_cast<Ztl_bstr_t*(__cdecl*)(Ztl_bstr_t*, int)>(0x004A8CE6)(&sActionName, nAction);

            // action and item id are packed into pAdditionalLayer->nData
            if ((pAdditionalLayer->nData & 0xFFFFFF) == nItemID && ((pAdditionalLayer->nData >> 24) & 0xFF) == nAction) {
                continue;
            }
            pAdditionalLayer->nData = (nAction << 24) | (nItemID & 0xFFFFFF);

            // resolve UOL
            IWzPropertyPtr pEffect = search->second;
            Ztl_variant_t vAction = pEffect->item[sActionName];
            wchar_t sUOL[1024];
            swprintf(sUOL, 1024, L"Effect/ItemEff.img/%d/effect/%ls", nItemID, vAction.vt == VT_EMPTY ? L"default" : sActionName.GetBSTR());

            // load layer and animate
            if (pUser->LoadLayer(sUOL, pAvatar->m_pLayerUnderFace->flip, pAdditionalLayer->l, nullptr)) {
                pAdditionalLayer->l.pLayer->Animate(GA_REPEAT);
            } else {
                pUser->RemoveAdditionalLayer(AL_ITEMEFFECT);
            }
            break;
        }
    }

    // set layer visibility
    if (pAdditionalLayer->l.pLayer) {
        pAdditionalLayer->l.pLayer->visible = bItemEff;
    }
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
    ATTACH_HOOK(CUser__OnAvatarModified, CUser__OnAvatarModified_hook);
    ATTACH_HOOK(CUser__SetMoveAction, CUser__SetMoveAction_hook);
}