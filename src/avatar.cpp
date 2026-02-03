#include "pch.h"
#include "hook.h"
#include "wvs/avatar.h"
#include "wvs/wvsapp.h"
#include "wvs/iteminfo.h"
#include "wvs/util.h"
#include <windows.h>

// Change tamingMob ID from 1983XXX -> 1939XXX
#define TAMINGMOB_ID_PREFIX 1939


void CAvatar::Constructor_hook() {
    CAvatar::Constructor(this);
    auto p = new CustomData{};
    m_pCustomData = p;
}

void CAvatar::Destructor_hook() {
    delete m_pCustomData;
    CAvatar::Destructor(this);
}

void CAvatar::RegisterNextBlink_hook() {
    m_pCustomData->bBlinking = 0;
    m_tNextBlink = CWvsApp::GetInstance()->m_tUpdateTime + (rand() % 3000) + 2000;
}

static auto CAvatar__Update_jmp1 = 0x004534CC;
static auto CAvatar__Update_ret1 = 0x004534D2;
void __declspec(naked) CAvatar__Update_hook1() {
    __asm {
        mov     ecx, [ ebx + 0x484 ]
        cmp     [ ecx ], edi
        jmp     [ CAvatar__Update_ret1 ]
    }
}

static auto CAvatar__Update_jmp2 = 0x00453612;
static auto CAvatar__Update_ret2 = 0x0045361C;
void __declspec(naked) CAvatar__Update_hook2() {
    __asm {
        mov     eax, [ ebx + 0x484 ]
        mov     [ eax ], 1
        jmp     [ CAvatar__Update_ret2 ]
    }
}


void CAvatar::SetRidingVehicle_hook(int nVehicleID) {
    if (!m_bForcingAppearance) {
        // CAvatar::ClearActionLayer(this, 0);
        reinterpret_cast<void(__thiscall*)(CAvatar*, int)>(0x00453A29)(this, 0);
        // Handle tamingMob chair
        m_nRidingVehicleID = nVehicleID;
        if (!nVehicleID && m_pCustomData->nRidingChairID) {
            m_nRidingVehicleID = m_pCustomData->nRidingChairID;
        }
        PrepareActionLayer(6, 100, 0);
    }
}

static auto load_tamingmob_action = reinterpret_cast<void(__cdecl*)(int, int, int, void*)>(0x00413A43);
void __cdecl load_tamingmob_action_hook(int nAction, int nMobBodyID, int nID, void* aFrame) {
    if (nMobBodyID / 1000 == TAMINGMOB_ID_PREFIX) {
        nAction = 39; // ACT_SIT
    }
    load_tamingmob_action(nAction, nMobBodyID, nID, aFrame);
}


class CUser {
public:
    MEMBER_AT(CAvatar, 0x88, m_CAvatar)

    MEMBER_HOOK(void, 0x00930B27, Update)
    MEMBER_HOOK(void, 0x0093C7C3, SetActivePortableChair, int nItemID)
};

void CUser::Update_hook() {
    CUser::Update(this);
    POINT p = m_CAvatar.m_pCustomData->ptBodyRelMove;
    if (p.x || p.y) {
        m_CAvatar.m_pBodyOrigin->RelMove(p.x, p.y);
    }
}

void CUser::SetActivePortableChair_hook(int nItemID) {
    CUser::SetActivePortableChair(this, nItemID);
    if (nItemID == 0) {
        m_CAvatar.m_pCustomData->ptBodyRelMove = {0, 0};
        if (m_CAvatar.m_nRidingVehicleID / 1000 == TAMINGMOB_ID_PREFIX) {
            m_CAvatar.m_pCustomData->nRidingChairID = 0;
            m_CAvatar.SetRidingVehicle_hook(0);
        }
        return;
    }
    IWzPropertyPtr pInfo = CItemInfo::GetInstance()->GetItemInfo(nItemID);
    if (!pInfo) {
        return;
    }
    // Resolve tamingMob property
    int nTamingMob = get_int32(pInfo->item[L"tamingMob"], 0);
    if (nTamingMob && nTamingMob / 1000 == TAMINGMOB_ID_PREFIX) {
        m_CAvatar.m_pCustomData->nRidingChairID = nTamingMob;
        m_CAvatar.SetRidingVehicle_hook(nTamingMob);
    }
    // Resolve bodyRelMove offsets
    IWzVector2DPtr pVector;
    IUnknownPtr pUnknown = pInfo->item[L"bodyRelMove"].GetUnknown();
    if (FAILED(pUnknown.QueryInterface(__uuidof(IWzVector2D), &pVector))) {
        return;
    }
    // Store to CAvatar->m_pCustomData struct
    m_CAvatar.m_pCustomData->ptBodyRelMove = {pVector->x, pVector->y};
}


void AttachAvatarDataMod() {
    // Inject CAvatar::CustomData struct to CAvatar
    ATTACH_HOOK(CAvatar::Constructor, CAvatar::Constructor_hook);
    ATTACH_HOOK(CAvatar::Destructor, CAvatar::Destructor_hook);
    ATTACH_HOOK(CAvatar::RegisterNextBlink, CAvatar::RegisterNextBlink_hook);
    PatchJmp(CAvatar__Update_jmp1, CAvatar__Update_hook1);
    PatchJmp(CAvatar__Update_jmp2, CAvatar__Update_hook2);

    // Implement tamingMob
    ATTACH_HOOK(CAvatar::SetRidingVehicle, CAvatar::SetRidingVehicle_hook);
    PatchCall(0x00413A31, load_tamingmob_action_hook); // CActionMan::LoadTamingMobAction

    // Implement bodyRelMove
    ATTACH_HOOK(CUser::Update, CUser::Update_hook);
    ATTACH_HOOK(CUser::SetActivePortableChair, CUser::SetActivePortableChair_hook);
}