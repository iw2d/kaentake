#include "hook.h"
#include "wvs/ctrlwnd.h"
#include "ztl/zalloc.h"
#include "ztl/zcom.h"
#include "ztl/tsingleton.h"

ZALLOC_GLOBAL
ZALLOCEX(ZAllocAnonSelector, 0x00BF0B00)


static ZRef<CCtrlComboBox> g_cbResolution;
static int g_nResolution = 0;

IWzGr2DPtr& get_gr() {
    return *reinterpret_cast<IWzGr2DPtr*>(0x00BF14EC);
}


class CConfig : public TSingleton<CConfig, 0x00BEBF9C> {
public:
    enum {
        GLOBAL_OPT = 0x0,
        LAST_CONNECT_INFO = 0x1,
        CHARACTER_OPT = 0x2,
    };

    int GetOpt_Int(int nType, const char* sKey, int nDefaultValue, int nLowBound, int nHighBound) {
        return reinterpret_cast<int(__thiscall*)(CConfig*, int, const char*, int, int, int)>(0x0049EF65)(this, nType, sKey, nDefaultValue, nLowBound, nHighBound);
    }
    void SetOpt_Int(int nType, const char* sKey, int nValue) {
        reinterpret_cast<void(__thiscall*)(CConfig*, int, const char*, int)>(0x0049EFB5)(this, nType, sKey, nValue);
    }
};

static auto CConfig__LoadGlobal = reinterpret_cast<void(__thiscall*)(CConfig*)>(0x0049C441);
void __fastcall CConfig__LoadGlobal_hook(CConfig* pThis, void* _EDX) {
    CConfig__LoadGlobal(pThis);
    g_nResolution = pThis->GetOpt_Int(CConfig::GLOBAL_OPT, "soScreenResolution", 0, 0, 4);
}

static auto CConfig__SaveGlobal = reinterpret_cast<void(__thiscall*)(CConfig*)>(0x0049C8E7);
void __fastcall CConfig__SaveGlobal_hook(CConfig* pThis, void* _EDX) {
    CConfig__SaveGlobal(pThis);
    pThis->SetOpt_Int(CConfig::GLOBAL_OPT, "soScreenResolution", g_nResolution);
}

static auto CConfig__ApplySysOpt = reinterpret_cast<void(__thiscall*)(CConfig*, void*, int)>(0x0049EA33);
void __fastcall CConfig__ApplySysOpt_hook(CConfig* pThis, void* _EDX, void* pSysOpt, int bApplyVideo) {
    CConfig__ApplySysOpt(pThis, pSysOpt, bApplyVideo);
    if (pSysOpt && bApplyVideo && g_cbResolution) {
        g_nResolution = g_cbResolution->m_nSelect;

        int nScreenWidth = 800;
        int nScreenHeight = 600;
        switch (g_nResolution) {
        case 1:
            nScreenWidth = 1024;
            nScreenHeight = 768;
            break;
        case 2:
            nScreenWidth = 1366;
            nScreenHeight = 768;
            break;
        case 3:
            nScreenWidth = 1600;
            nScreenHeight = 900;
            break;
        case 4:
            nScreenWidth = 1920;
            nScreenHeight = 1080;
            break;
        }

        IWzGr2D_DX9Ptr gr = get_gr()->GetInner();
        if (SUCCEEDED(gr->screenResolution(nScreenWidth, nScreenHeight))) {
            int nAdjustCenterY = (nScreenHeight - 600) / 2;
            gr->AdjustCenter(0, -nAdjustCenterY);
        }
    }
}


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
    g_cbResolution->SetSelect(g_nResolution);
}

static auto CUISysOpt__dtor = reinterpret_cast<void(__thiscall*)(CUISysOpt*)>(0x007FF4AA);
void __fastcall CUISysOpt__dtor_hook(CUISysOpt* pThis) {
    CUISysOpt__dtor(pThis);
    g_cbResolution = nullptr;
}


void AttachResolutionMod() {
    ATTACH_HOOK(CConfig__LoadGlobal, CConfig__LoadGlobal_hook);
    ATTACH_HOOK(CConfig__SaveGlobal, CConfig__SaveGlobal_hook);
    ATTACH_HOOK(CConfig__ApplySysOpt, CConfig__ApplySysOpt_hook);
    ATTACH_HOOK(CUISysOpt__OnCreate, CUISysOpt__OnCreate_hook);
    ATTACH_HOOK(CUISysOpt__dtor, CUISysOpt__dtor_hook);
    Patch4(0x00994525 + 1, 0); // Hide Monster Info in CUISysOpt
}