#pragma once
#include "hook.h"
#include "ztl/ztl.h"


class CAvatar {
public:
    struct CustomData {
        int bBlinking;
        POINT ptBodyRelMove;
    };

    virtual ~CAvatar() = 0;
    virtual int CanUseBaseHand() = 0;
    virtual int IsEvanJob() = 0;
    virtual void OnAvatarModified() = 0;
    virtual void SetMoveAction(int nMA, int bReload) = 0;
    virtual void PrepareActionLayer(int nActionSpeed, int nWalkSpeed, int bKeyDown) = 0;

    MEMBER_AT(CustomData*, 0x484, m_pCustomData) // Hijack m_bBlinking
    MEMBER_AT(int, 0x488, m_tNextBlink)
    MEMBER_AT(IWzVector2DPtr, 0x10B8, m_pBodyOrigin)

    // avatar.cpp
    MEMBER_HOOK(void, 0x0044FE6C, Constructor)
    MEMBER_HOOK(void, 0x0045011C, Destructor)
    MEMBER_HOOK(void, 0x00453AA2, RegisterNextBlink)
};