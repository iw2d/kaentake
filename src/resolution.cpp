#include "hook.h"
#include "ztl/zalloc.h"
#include <WzLib/IWzGr2D.h>
#include <WzLib/IWzGr2D_DX9.h>

ZALLOC_GLOBAL
ZALLOCEX(ZAllocAnonSelector, 0x00BF0B00)


class CUISysOpt;

class IGObj {
public:
    virtual void Update() {}
};

class IUIMsgHandler {
public:
    virtual void OnKey(unsigned int wParam, unsigned int lParam) {}
    // TODO
};


class CCtrlWnd : public IGObj, public IUIMsgHandler, public ZRefCounted {
};

class CCtrlComboBox : public CCtrlWnd {
public:
    unsigned char pad0[0x10C - sizeof(CCtrlWnd)];

    struct CREATEPARAM {
        unsigned char pad0[0x50];
        MEMBER_AT(int, 0x0, nBackColor)
        MEMBER_AT(int, 0x4, nBackFocusedColor)
        MEMBER_AT(int, 0x8, nBorderColor)

        CREATEPARAM() {
            reinterpret_cast<void(__thiscall*)(CREATEPARAM*)>(0x004B620C)(this);
        }
        ~CREATEPARAM() {
            reinterpret_cast<void(__thiscall*)(CREATEPARAM*)>(0x004B63AE)(this);
        }
    };

    CCtrlComboBox() {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*)>(0x004C4259)(this);
    }
    ~CCtrlComboBox() {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*)>(0x004C4372)(this);
    }
    void CreateCtrl(void* pParent, unsigned int nId, int nType, int l, int t, int w, int h, void* pData) {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*, void*, unsigned int, int, int, int, int, int, void*)>(0x004C446E)(this, pParent, nId, nType, l, t, w, h, pData);
    }
    void AddItem(const char* sItemName, unsigned int dwParam) {
        reinterpret_cast<void(__thiscall*)(CCtrlComboBox*, const char*, unsigned int)>(0x004C6DD6)(this, sItemName, dwParam);
    }
};

static ZRef<CCtrlComboBox> g_cbResolution;



static auto CUISysOpt__OnCreate = reinterpret_cast<void(__thiscall*)(CUISysOpt*, void*)>(0x00994163);
void __fastcall CUISysOpt__OnCreate_hook(CUISysOpt* pThis, void* _EDX, void* pData) {
    CUISysOpt__OnCreate(pThis, pData);

    CCtrlComboBox::CREATEPARAM paramComboBox;
    paramComboBox.nBackColor = 0xFFEEEEEE;
    paramComboBox.nBackFocusedColor = 0xFFA5A198;
    paramComboBox.nBorderColor = 0xFF999999;

    g_cbResolution = new CCtrlComboBox();
    g_cbResolution->CreateCtrl(pThis, 2000, 0, 65, 58, 174, 18, &paramComboBox);
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
}