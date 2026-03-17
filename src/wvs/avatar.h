#pragma once
#include "hook.h"
#include "ztl/ztl.h"
#include <windows.h>


#pragma pack(push, 1)
struct AvatarLook : public ZRefCounted {
    unsigned char nGender;
    int nSkin;
    int nFace;
    int nWeaponStickerID;
    int anHairEquip[60];
    int anUnseenEquip[60];
    int anPetID[3];
};
#pragma pack(pop)
static_assert(sizeof(AvatarLook) == 0x205);


struct USERLAYER {
    enum POSTYPE {
        POS_BODY_ORIGIN = 0x0,
        POS_FACE_ORIGIN = 0x1,
        POS_CENTER = 0x2,
        POS_GROUND_ORIGIN = 0x3,
    };

    int bFixed;
    POSTYPE nPos;
    IWzGr2DLayerPtr pLayer;
};
static_assert(sizeof(USERLAYER) == 0xC);

struct ITEMEFFECTLAYER {
    int nItemID;
    int nAction;
    int bFlip;
    USERLAYER l;

    inline void Reset() {
        nItemID = 0;
        nAction = 0;
        l.pLayer = nullptr;
    }
};


class CAvatar {
public:
    struct CustomData {
        int bBlinking;
        POINT ptBodyRelMove;
        int nRidingChairID;
        int nReplacedStandAction;
        ITEMEFFECTLAYER aItemEffectLayer[60];
    };

    virtual ~CAvatar() = 0;
    virtual int CanUseBaseHand() = 0;
    virtual int IsEvanJob() = 0;
    virtual void OnAvatarModified() = 0;
    virtual void SetMoveAction(int nMA, int bReload) = 0;
    virtual void PrepareActionLayer(int nActionSpeed, int nWalkSpeed, int bKeyDown) = 0;

    MEMBER_AT(AvatarLook, 0x4, m_avatarLook)
    MEMBER_AT(int, 0x460, m_bForcingAppearance)
    MEMBER_AT(CustomData*, 0x484, m_pCustomData) // Hijack m_bBlinking
    MEMBER_AT(int, 0x488, m_tNextBlink)
    MEMBER_AT(int, 0x4BC, m_nRidingVehicleID)
    MEMBER_AT(int, 0x4EC, m_nOneTimeAction)
    MEMBER_AT(IWzVector2DPtr, 0x10B8, m_pBodyOrigin)
    MEMBER_AT(IWzGr2DLayerPtr, 0x10C8, m_pLayerUnderFace)

    // avatar.cpp
    MEMBER_HOOK(void, 0x0044FE6C, Constructor)
    MEMBER_HOOK(void, 0x0045011C, Destructor)
    MEMBER_HOOK(void, 0x00453AA2, RegisterNextBlink)
    MEMBER_HOOK(void, 0x00456E35, SetRidingVehicle, int nVehicleID)
    MEMBER_HOOK(int, 0x00451EC8, MoveAction2RawAction, int nMA, int* pnDir)

    inline int GetCurrentAction(int* pnDir) {
        return reinterpret_cast<int(__thiscall*)(CAvatar*, int*)>(0x00451E4C)(this, pnDir);
    }
    inline void ClearActionLayer(int nIndex) {
        reinterpret_cast<void(__thiscall*)(CAvatar*, int)>(0x00453A29)(this, nIndex);
    }
};