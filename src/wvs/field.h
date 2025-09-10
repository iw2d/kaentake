#pragma once
#include "hook.h"
#include "wvs/stage.h"


class CMapLoadable : public CStage {
public:
    MEMBER_AT(IWzPropertyPtr, 0x2C, m_pPropFieldInfo)
    MEMBER_AT(RECT, 0xF0, m_rcViewRange)
    MEMBER_HOOK(void, 0x00641EF1, RestoreViewRange)

    void ReloadBack() {
        reinterpret_cast<void(__thiscall*)(CMapLoadable*)>(0x00644491)(this);
    }
};


class CField : public CMapLoadable {
};


CField* get_field() {
    return reinterpret_cast<CField*(__cdecl*)()>(0x00437A0C)();
}