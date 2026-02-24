#include "pch.h"
#include "hook.h"
#include "wvs/util.h"
#include "ztl/ztl.h"


static IWzCanvasPtr g_pCanvasHpBack;
static IWzPropertyPtr g_pPropHpNum;

class CField {
public:
    MEMBER_AT(IWzGr2DLayerPtr, 0x1E4, m_pLayerHPTag)
    MEMBER_HOOK(void, 0x005336CA, ShowMobHPTag, unsigned int dwMobID, int nColor, int nBgColor, int nHP, int nMaxHP)
};

static auto draw_text_by_image = reinterpret_cast<int(__cdecl*)(IWzCanvasPtr pCanvas, int nLeft, int nTop, ZXString<char> sText, IWzPropertyPtr pBase, int nHorzSpace)>(0x0098812C);

void CField::ShowMobHPTag_hook(unsigned int dwMobID, int nColor, int nBgColor, int nHP, int nMaxHP) {
    CField::ShowMobHPTag(this, dwMobID, nColor, nBgColor, nHP, nMaxHP);
    if (!m_pLayerHPTag) {
        return;
    }
    IWzCanvasPtr pCanvas = m_pLayerHPTag->canvas[0];

    // Draw background
    if (!g_pCanvasHpBack) {
        g_pCanvasHpBack = get_unknown(get_rm()->GetObjectA(L"Custom/UIWindowEx.img/MobHPRatioInfo/mob/0/backgrnd"));
    }
    pCanvas->Copy(0, 37, g_pCanvasHpBack);

    // Draw hp percentage
    if (!g_pPropHpNum) {
        g_pPropHpNum = get_rm()->GetObjectA(L"Custom/UIWindowEx.img/MobHPRatioInfo/num").GetUnknown();
    }
    char sRatio[20];
    float fRatio = static_cast<float>(nHP) / static_cast<float>(nMaxHP);
    if (fRatio > 0.99f) {
        sprintf_s(sRatio, 20, "100%%");
    } else {
        sprintf_s(sRatio, 20, "%.1f%%", fRatio * 100);
    }
    int nRatioLeft = fRatio > 0.99f ? 7 : (fRatio >= 0.1f ? 5 : 11);
    draw_text_by_image(pCanvas, nRatioLeft, 42, sRatio, g_pPropHpNum, 1);
}

unsigned int __fastcall CField__ShowMobHPTag__GetHeight_hook(IWzCanvas* pThis, void* _EDX) {
    return 54;
}


void AttachMobHpTagMod() {
    ATTACH_HOOK(CField::ShowMobHPTag, CField::ShowMobHPTag_hook);
    PatchCall(0x00533AFD, &CField__ShowMobHPTag__GetHeight_hook); // patch m_pLayerHPTag height in CField::ShowMobHPTag
}