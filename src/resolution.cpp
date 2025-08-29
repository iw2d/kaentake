#include "hook.h"
#include "wvs/ctrlwnd.h"
#include "ztl/zalloc.h"

ZALLOC_GLOBAL
ZALLOCEX(ZAllocAnonSelector, 0x00BF0B00)


static ZRef<CCtrlComboBox> g_cbResolution;

class CUISysOpt;

static auto CUISysOpt__OnCreate = reinterpret_cast<void(__thiscall*)(CUISysOpt*, void*)>(0x00994163);
void __fastcall CUISysOpt__OnCreate_hook(CUISysOpt* pThis, void* _EDX, void* pData) {
    CUISysOpt__OnCreate(pThis, pData);

    CCtrlComboBox::CREATEPARAM paramComboBox;
    paramComboBox.nBackColor = 0xFFEEEEEE;
    paramComboBox.nBackFocusedColor = 0xFFA5A198;
    paramComboBox.nBorderColor = 0xFF999999;

    g_cbResolution = new CCtrlComboBox();
    g_cbResolution->CreateCtrl(pThis, 2000, 0, 67, 278, 175, 18, &paramComboBox);
    const char* asResolution[] = {
        "800 x 600",
        "1024 x 768",
        "1366 x 768",
        "1600 x 900",
        "1920 x 1080",
    };
    unsigned int dwResolutionParam = 0;
    for (auto sResolution : asResolution) {
        g_cbResolution->AddItem(sResolution, dwResolutionParam++);
    }
}

static auto CUISysOpt__dtor = reinterpret_cast<void(__thiscall*)(CUISysOpt*)>(0x007FF4AA);
void __fastcall CUISysOpt__dtor_hook(CUISysOpt* pThis) {
    CUISysOpt__dtor(pThis);
    g_cbResolution = nullptr;
}


void AttachResolutionMod() {
    ATTACH_HOOK(CUISysOpt__OnCreate, CUISysOpt__OnCreate_hook);
    ATTACH_HOOK(CUISysOpt__dtor, CUISysOpt__dtor_hook);
    Patch4(0x00994525 + 1, 0); // Hide Monster Info in CUISysOpt
}