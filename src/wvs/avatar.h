#pragma once
#include "hook.h"
#include "ztl/ztl.h"
#include <windows.h>


class CAvatar {
public:
    struct CustomData {
        int bBlinking;
        POINT ptBodyRelMove;
        int nRidingChairID;
    };

    virtual ~CAvatar() = 0;
    virtual int CanUseBaseHand() = 0;
    virtual int IsEvanJob() = 0;
    virtual void OnAvatarModified() = 0;
    virtual void SetMoveAction(int nMA, int bReload) = 0;
    virtual void PrepareActionLayer(int nActionSpeed, int nWalkSpeed, int bKeyDown) = 0;

    MEMBER_AT(int, 0x460, m_bForcingAppearance)
    MEMBER_AT(CustomData*, 0x484, m_pCustomData) // Hijack m_bBlinking
    MEMBER_AT(int, 0x488, m_tNextBlink)
    MEMBER_AT(int, 0x4BC, m_nRidingVehicleID)
    MEMBER_AT(IWzVector2DPtr, 0x10B8, m_pBodyOrigin)

    // avatar.cpp
    MEMBER_HOOK(void, 0x0044FE6C, Constructor)
    MEMBER_HOOK(void, 0x0045011C, Destructor)
    MEMBER_HOOK(void, 0x00453AA2, RegisterNextBlink)
    MEMBER_HOOK(void, 0x00456E35, SetRidingVehicle, int nVehicleID)
};