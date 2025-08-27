#include "hook.h"
#include <WzLib/IWzGr2D.h>
#include <WzLib/IWzGr2D_DX9.h>


IWzGr2DPtr& get_gr() {
    return *reinterpret_cast<IWzGr2DPtr*>(0x00BF14EC);
}


static auto CWvsApp__InitializeGr2D = reinterpret_cast<void(__thiscall*)(void*)>(0x009F7A3B);

void __fastcall CWvsApp__InitializeGr2D_hook(void* pThis, void* _EDX) {
    CWvsApp__InitializeGr2D(pThis);
    IWzGr2D_DX9Ptr pGr2D_DX9 = get_gr()->GetInner();
    pGr2D_DX9->screenResolution(1024, 768);
    pGr2D_DX9->AdjustCenter(0, -84);
}


void AttachResolutionMod() {
    ATTACH_HOOK(CWvsApp__InitializeGr2D, CWvsApp__InitializeGr2D_hook);
}