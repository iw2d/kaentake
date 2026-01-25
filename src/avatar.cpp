#include "pch.h"
#include "hook.h"
#include "wvs/avatar.h"
#include "wvs/wvsapp.h"
#include "wvs/iteminfo.h"


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


class CUser {
public:
    MEMBER_AT(CAvatar, 0x88, m_Avatar)

    MEMBER_HOOK(void, 0x00930B27, Update)
    MEMBER_HOOK(void, 0x0093C7C3, SetActivePortableChair, int nItemID)
};

void CUser::Update_hook() {
    CUser::Update(this);
    POINT p = m_Avatar.m_pCustomData->ptBodyRelMove;
    if (p.x || p.y) {
        m_Avatar.m_pBodyOrigin->RelMove(p.x, p.y);
    }
}

void CUser::SetActivePortableChair_hook(int nItemID) {
    CUser::SetActivePortableChair(this, nItemID);
    if (nItemID == 0) {
        m_Avatar.m_pCustomData->ptBodyRelMove = {0, 0};
        return;
    }
    // Resolve bodyRelMove offsets
    IWzPropertyPtr pInfo = CItemInfo::GetInstance()->GetItemInfo(nItemID);
    if (!pInfo) {
        return;
    }
    IWzVector2DPtr pVector;
    IUnknownPtr pUnknown = pInfo->item[L"bodyRelMove"].GetUnknown();
    if (FAILED(pUnknown.QueryInterface(__uuidof(IWzVector2D), &pVector))) {
        return;
    }
    // Store to CAvatar->m_pCustomData struct
    m_Avatar.m_pCustomData->ptBodyRelMove = {pVector->x, pVector->y};
}


void AttachAvatarDataMod() {
    // Inject CAvatar::CustomData struct to CAvatar
    ATTACH_HOOK(CAvatar::Constructor, CAvatar::Constructor_hook);
    ATTACH_HOOK(CAvatar::Destructor, CAvatar::Destructor_hook);
    ATTACH_HOOK(CAvatar::RegisterNextBlink, CAvatar::RegisterNextBlink_hook);
    PatchJmp(CAvatar__Update_jmp1, CAvatar__Update_hook1);
    PatchJmp(CAvatar__Update_jmp2, CAvatar__Update_hook2);

    // Implement bodyRelMove
    ATTACH_HOOK(CUser::Update, CUser::Update_hook);
    ATTACH_HOOK(CUser::SetActivePortableChair, CUser::SetActivePortableChair_hook);
}