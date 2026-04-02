#include "pch.h"
#include "hook.h"
#include "wvs/tempstat.h"
#include "wvs/statusbar.h"
#include "wvs/util.h"
#include "ztl/ztl.h"


static IWzPropertyPtr g_pPropMinute;
static IWzPropertyPtr g_pPropSecond;

void DrawDuration(IWzCanvasPtr pCanvas, int nSeconds, int nX = 0, int nY = 0) {
    if (!g_pPropMinute) {
        g_pPropMinute = get_rm()->GetObjectA(L"UI/UIWindowEx.img/SkillCooldownNumber/0").GetUnknown();
    }
    if (!g_pPropSecond) {
        g_pPropSecond = get_rm()->GetObjectA(L"UI/UIWindowEx.img/SkillCooldownNumber/2").GetUnknown();
    }
    IWzPropertyPtr pBase;
    int nValue;
    int nOffsetX;
    int nOffsetY;
    if (nSeconds >= 60) {
        pBase = g_pPropMinute;
        nValue = nSeconds / 60;
        nOffsetX = 2;
        nOffsetY = 19;
    } else {
        pBase = g_pPropSecond;
        nValue = nSeconds;
        nOffsetX = nValue >= 10 ? 6 : 11;
        nOffsetY = 10;
    }
    if (nSeconds > 0 && nValue < 999) {
        // draw_number_by_image(pCanvas, nOffsetX, nOffsetY, nValue, pBase, 0);
        reinterpret_cast<int(__cdecl*)(IWzCanvasPtr, int, int, int, IWzPropertyPtr, int)>(0x00988345)(pCanvas, nX + nOffsetX, nY + nOffsetY, nValue, pBase, 0);
    }
}


static auto TEMPORARY_STAT__UpdateShadowIndex = 0x007B44F4;
void __fastcall TEMPORARY_STAT__UpdateShadowIndex_hook(CTemporaryStatView::TEMPORARY_STAT* pThis, void* _EDX) {
    if (pThis->bNoShadow) {
        return;
    }
    int nSeconds = pThis->tLeft / 1000;
    if (nSeconds == pThis->nIndexShadow) {
        return;
    }
    pThis->nIndexShadow = nSeconds; // hijack nIndexShadow to redraw every second

    int nShadowIndex = 0;
    if (pThis->tLeftUnit) {
        nShadowIndex = pThis->tLeft / pThis->tLeftUnit;
        if (nShadowIndex < 0) {
            nShadowIndex = 0;
        } else if (nShadowIndex > 15) {
            nShadowIndex = 15;
        }
    }

    pThis->pLayerShadow->RemoveCanvas(-2);
    IWzCanvasPtr pCanvas;
    PcCreateObject<IWzCanvasPtr>(L"Canvas", pCanvas, nullptr);
    pCanvas->Create(32, 32);
    pCanvas->Copy(0, 0, CUIStatusBar::GetInstance()->m_aCanvasSkillCooltime[nShadowIndex]);

    if (pThis->nID != 5221006) {
        DrawDuration(pCanvas, nSeconds);
    }
    pThis->pLayerShadow->InsertCanvas(pCanvas, 500, 210, 64);
}


struct CUIStatusBar__DrawSkillCooltime_stack {
    MEMBER_AT(int, 0x64, nCanvasX)
    MEMBER_AT(int, 0x68, nCanvasY)
    MEMBER_AT(int*, 0x74, pnLastIndex)
    MEMBER_AT(IWzCanvas*, 0x7C, pCanvas)
};

void __stdcall CUIStatusBar__DrawSkillCooltime_helper(CUIStatusBar__DrawSkillCooltime_stack* p, int nSeconds, int nIndex) {
    // hijack pnLastIndex (m_aFuncKeyMappedSkillCooltime[i]) to store nSeconds
    if (nIndex < 0 || *p->pnLastIndex == nSeconds) {
        return;
    }
    *p->pnLastIndex = nSeconds;
    p->pCanvas->Copy(p->nCanvasX, p->nCanvasY, CUIStatusBar::GetInstance()->m_aCanvasSkillCooltime[nIndex]);
    DrawDuration(p->pCanvas, nSeconds, p->nCanvasX, p->nCanvasY);
}

static auto CUIStatusBar__DrawSkillCooltime_ret = 0x008E0990;
void __declspec(naked) CUIStatusBar__DrawSkillCooltime_hook() {
    __asm {
        push    esi                                     ; computed shadow index
        push    ebx                                     ; remaining cooltime in seconds
        lea     eax, [ ebp - 0x8C ]                     ; starting address of stack
        push    eax
        call    CUIStatusBar__DrawSkillCooltime_helper
        jmp     [ CUIStatusBar__DrawSkillCooltime_ret ]
    }
}


void AttachTempStatMod() {
    ATTACH_HOOK(TEMPORARY_STAT__UpdateShadowIndex, TEMPORARY_STAT__UpdateShadowIndex_hook);
    PatchJmp(0x008E085B, &CUIStatusBar__DrawSkillCooltime_hook); // CUIStatusBar::DrawSkillCooltime
}