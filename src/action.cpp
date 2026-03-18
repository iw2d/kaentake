#include "pch.h"
#include "hook.h"
#include "ztl/ztl.h"
#include <map>

#define TAMINGMOB_ACTION_OFFSET 0x520 - 4
#define CHARACTER_ACTION_COUNT  162 + 2
#define ACT_STAND1_FLOATING     162
#define ACT_STAND2_FLOATING     163


struct ACTIONDATA {
    struct PIECE {
        int nAction;
        int nFrameIdx;
        int tFrameDelay;
        int bShowFace;
        int bFlip;
        int nRotate;
        POINT ptMove;
    };
    static_assert(sizeof(PIECE) == 0x20);

    Ztl_bstr_t bsName;
    int bZigZag;
    int bPieced;
    int tTotalDelay;
    int tEventDelay;
    ZArray<ACTIONDATA::PIECE> aPiece;
};
static_assert(sizeof(ACTIONDATA) == 0x18);

static ACTIONDATA g_aCharacterActionData[CHARACTER_ACTION_COUNT];


class CActionMan {
public:
    MEMBER_HOOK(void, 0x00406ABD, Init)
};

void CActionMan::Init_hook() {
    // Patch all usages of s_aCharacterActionData
    for (auto i = 0; i < sizeof(ACTIONDATA); i += 4) {
        char sPattern[12];
        uintptr_t uAddress = 0x00BEC620 + i; // s_aCharacterActionData
        sprintf_s(sPattern, sizeof(sPattern), "%02X %02X %02X %02X", uAddress & 0xFF, (uAddress >> 8) & 0xFF, (uAddress >> 16) & 0xFF, (uAddress >> 24) & 0xFF);
        uintptr_t pValue = reinterpret_cast<uintptr_t>(&g_aCharacterActionData) + i;
        PatchAllByPattern(reinterpret_cast<void*>(0x00401000), reinterpret_cast<void*>(0x00E92000), sPattern, &pValue, sizeof(uintptr_t));
    }

    // Populate g_aCharacterActionData
    memcpy(g_aCharacterActionData, reinterpret_cast<void*>(0x00BEC620), sizeof(ACTIONDATA) * 162);
    g_aCharacterActionData[ACT_STAND1_FLOATING] = ACTIONDATA{ L"stand1_floating", 0, 1 };
    g_aCharacterActionData[ACT_STAND2_FLOATING] = ACTIONDATA{ L"stand2_floating", 0, 1 };
    Patch4(0x004A8D31 + 2, reinterpret_cast<uintptr_t>(&g_aCharacterActionData[CHARACTER_ACTION_COUNT])); // get_action_code_from_name

    // Continue with initialization
    CActionMan::Init(this);
}


class CAvatar {
public:
    struct ACTIONINFO {
        MEMBER_AT(int, 0x0, nCurFrameIndex)
        MEMBER_AT(int, 0x4, tCurFrameRemain)
        MEMBER_AT(int, 0x8, tTotFrameDelay)
        MEMBER_AT(void*, 0xC, aFrameDelay) // ZArray<long>
        MEMBER_ARRAY_AT(void*, 0x10, aaAction, CHARACTER_ACTION_COUNT) // MAX_COUNT = (TAMINGMOB_ACTION_OFFSET - 0x10) / 4 = 323
        MEMBER_AT(void**, TAMINGMOB_ACTION_OFFSET, pTamingMobAction)   // convert to pointer
        MEMBER_ARRAY_AT(void*, 0x520, aaMorphAction, 46)
        MEMBER_AT(void*, 0x5D8, aSPAction) // ZArray<ZList<ZRef<CActionMan::SHADOWPARTNERACTIONFRAMEENTRY>>>

        MEMBER_HOOK(void, 0x0044FFE2, Constructor)
        MEMBER_HOOK(void, 0x00450093, Destructor)
    };
};

void CAvatar::ACTIONINFO::Constructor_hook() {
    nCurFrameIndex = -1;
    tCurFrameRemain = 0;
    tTotFrameDelay = 0;
    aFrameDelay = PVOID_NULLPTR;
    for (auto i = 0; i < CHARACTER_ACTION_COUNT; ++i) {
        aaAction[i] = nullptr;
    }
    auto aaTamingMobAction = new void*[CHARACTER_ACTION_COUNT];
    for (auto i = 0; i < CHARACTER_ACTION_COUNT; ++i) {
        aaTamingMobAction[i] = nullptr;
    }
    pTamingMobAction = aaTamingMobAction;
    for (auto i = 0; i < 46; ++i) {
        aaMorphAction[i] = nullptr;
    }
    aSPAction = PVOID_NULLPTR;
}

void CAvatar::ACTIONINFO::Destructor_hook() {
    // ZArray<ZList<ZRef<CActionMan::SHADOWPARTNERACTIONFRAMEENTRY>>>::RemoveAll
    reinterpret_cast<void(__thiscall*)(void*)>(0x004573B7)(&aSPAction);
    for (auto i = 0; i < 46; ++i) {
        // ZArray<ZRef<CActionMan::MORPHACTIONFRAMEENTRY>>::~ZArray<ZRef<CActionMan::MORPHACTIONFRAMEENTRY>>
        reinterpret_cast<void(__thiscall*)(void*)>(0x0045722A)(&aaMorphAction[i]);
    }
    for (auto i = 0; i < CHARACTER_ACTION_COUNT; ++i) {
        // ZArray<ZRef<CActionMan::TAMINGMOBACTIONFRAMEENTRY>>::~ZArray<ZRef<CActionMan::TAMINGMOBACTIONFRAMEENTRY>>
        reinterpret_cast<void(__thiscall*)(void*)>(0x00457240)(&pTamingMobAction[i]);
    }
    delete pTamingMobAction;
    for (auto i = 0; i < CHARACTER_ACTION_COUNT; ++i) {
        // ZArray<ZRef<CActionMan::CHARACTERACTIONFRAMEENTRY>>::~ZArray<ZRef<CActionMan::CHARACTERACTIONFRAMEENTRY>>
        reinterpret_cast<void(__thiscall*)(void*)>(0x00457235)(&aaAction[i]);
    }
    // ZArray<long>::RemoveAll
    reinterpret_cast<void(__thiscall*)(void*)>(0x00457245)(&aFrameDelay);
}


static auto CAvatar__Update_ret = 0x004527AA;
void __declspec(naked) CAvatar__Update_hook() {
    // Original: lea eax, [esi+edi*4+0x298]
    __asm {
        mov     eax, [ esi + TAMINGMOB_ACTION_OFFSET ]
        lea     eax, [ eax + edi * 4 ]
        jmp     [ CAvatar__Update_ret ]
    }
}

static auto CAvatar__ClearActionLayer_ret = 0x00453A68;
void __declspec(naked) CAvatar__ClearActionLayer_hook() {
    // Original: lea edi, [esi+0x298]
    __asm {
        mov     edi, [ esi + TAMINGMOB_ACTION_OFFSET ]
        jmp     [ CAvatar__ClearActionLayer_ret ]
    }
}

static auto CAvatar__PrepareActionLayer_ret = 0x0045456F;
void __declspec(naked) CAvatar__PrepareActionLayer_hook() {
    // Original: lea eax, [eax+ebx*4+0x298]
    __asm {
        mov     eax, [ eax + TAMINGMOB_ACTION_OFFSET ]
        lea     eax, [ eax + ebx * 4 ]
        jmp     [ CAvatar__PrepareActionLayer_ret ]
    }
}


void AttachCustomActionMod() {
    ATTACH_HOOK(CActionMan::Init, CActionMan::Init_hook);
    Patch4(0x004073A2 + 2, CHARACTER_ACTION_COUNT); // CActionMan::Init
    Patch4(0x0040ACDA + 1, CHARACTER_ACTION_COUNT); // CActionMan::GetWeaponAfterImage

    ATTACH_HOOK(CAvatar::ACTIONINFO::Constructor, CAvatar::ACTIONINFO::Constructor_hook);
    ATTACH_HOOK(CAvatar::ACTIONINFO::Destructor, CAvatar::ACTIONINFO::Destructor_hook);

    Patch4(0x00451010 + 1, CHARACTER_ACTION_COUNT); // CAvatar::Init
    Patch4(0x004522F4 + 3, CHARACTER_ACTION_COUNT); // CAvatar::Update
    Patch4(0x00453A3F + 1, CHARACTER_ACTION_COUNT); // CAvatar::ClearActionLayer
    Patch4(0x00453B2B + 2, CHARACTER_ACTION_COUNT); // CAvatar::PrepareActionLayer

    PatchJmp(0x004527A3, &CAvatar__Update_hook);
    PatchJmp(0x00453A62, &CAvatar__ClearActionLayer_hook);
    PatchJmp(0x00454568, &CAvatar__PrepareActionLayer_hook);
}