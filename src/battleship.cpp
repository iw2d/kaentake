#include "pch.h"
#include "hook.h"
#include "wvs/wnd.h"
#include "wvs/config.h"
#include "wvs/util.h"
#include "ztl/ztl.h"

#define UI_BATTLESHIP 20 // Hijack UI_ENERGYBAR - CUIWnd positions are saved in static arrays


class CUIBattleship : public CUIWnd {
public:
    int m_nHpLeft;
    int m_nHpMax;

    virtual ~CUIBattleship() = default;
    CUIBattleship() : CUIWnd(UI_BATTLESHIP, 0, -1, -1, 0, 0, 0) {
        int x, y;
        CConfig::GetInstance()->GetUIWndPos_hook(UI_BATTLESHIP, &x, &y, &m_nOption);
        CreateWnd_hook(x, y, 0, 0, 10, 1, nullptr, 1);
        m_width = 82;
        m_height = 10;
        InvalidateRect(nullptr);
    }

    void SetHp(int nHpLeft, int nHpMax) {
        if (nHpLeft == m_nHpLeft && nHpMax == m_nHpMax) {
            return;
        }
        m_nHpLeft = nHpLeft;
        m_nHpMax = nHpMax;

        IWzCanvasPtr pCanvas;
        PcCreateObject<IWzCanvasPtr>(L"Canvas", pCanvas, nullptr);
        pCanvas->Create(m_width, m_height);
        pCanvas->cx = 0;
        pCanvas->cy = 0;

        IWzPropertyPtr pProp = get_rm()->GetObjectA(L"UI/UIWindow.img/EnergyBar").GetUnknown();
        IWzCanvasPtr pCanvasW = get_unknown(pProp->item[L"w"]);
        IWzCanvasPtr pCanvasC = get_unknown(pProp->item[L"c"]);
        IWzCanvasPtr pCanvasE = get_unknown(pProp->item[L"e"]);

        pCanvas->Copy(0, 0, pCanvasW, 180);
        pCanvas->Copy(m_width - 6, 0, pCanvasE, 180);
        for (int x = 6; x < m_width - 6; x += 1) {
            pCanvas->Copy(x, 0, pCanvasC, 180);
        }

        if (m_nHpLeft > 0) {
            IWzCanvasPtr pGauge0 = get_unknown(get_rm()->GetObjectA(L"UI/UIWindow.img/EnergyBar/Gage/1/0"));
            IWzCanvasPtr pGauge1 = get_unknown(get_rm()->GetObjectA(L"UI/UIWindow.img/EnergyBar/Gage/1/1"));

            int nHpWidth = (int)((double)m_nHpLeft / (double)m_nHpMax * 78.0);
            for (int x = 2; x < nHpWidth + 2; x += 1) {
                if (x == 2 || x == m_width - 3) {
                    pCanvas->Copy(x, 3, pGauge0, 255);
                } else {
                    pCanvas->Copy(x, 2, pGauge1, 255);
                }
            }

            GetLayer()->RemoveCanvas(-2);
            GetLayer()->InsertCanvas(pCanvas);
        } else {
            GetLayer()->RemoveCanvas(-2);
        }
    }
};

static ZRef<CUIBattleship> g_pUIBattleship;


void __fastcall CTemporaryStatView__UpdatePassively_hook(void* pThis, void* _EDX, int nID, int nLeft, int nMax) {
    // CTemporaryStatView::UpdatePassively
    reinterpret_cast<void(__thiscall*)(void*, int, int, int)>(0x007B30DB)(pThis, nID, nLeft, nMax);
    if (!g_pUIBattleship) {
        g_pUIBattleship = new CUIBattleship();
    }
    g_pUIBattleship->SetHp(nLeft, nMax);
}


class CWvsContext : public TSingleton<CWvsContext, 0x00BE7918> {
public:
    MEMBER_HOOK(void, 0x00A041FF, OnLeaveGame)
    MEMBER_HOOK(void, 0x00A04920, ClearFieldUI)
    MEMBER_HOOK(void, 0x00A123A4, SetSkillCooltimeOver, int nSkillID, int tTimeOver)
};

void CWvsContext::OnLeaveGame_hook() {
    if (g_pUIBattleship) {
        g_pUIBattleship->Destroy();
        g_pUIBattleship = nullptr;
    }
    CWvsContext::OnLeaveGame(this);
}

void CWvsContext::ClearFieldUI_hook() {
    if (g_pUIBattleship) {
        g_pUIBattleship->Destroy();
        g_pUIBattleship = nullptr;
    }
    CWvsContext::ClearFieldUI(this);
}

void CWvsContext::SetSkillCooltimeOver_hook(int nSkillID, int tTimeOver) {
    CWvsContext::SetSkillCooltimeOver(this, nSkillID, tTimeOver);
    if (nSkillID == 5221006 && g_pUIBattleship) {
        // there are definitely better ways to handle this
        g_pUIBattleship->Destroy();
        g_pUIBattleship = nullptr;
    }
}


void AttachBattleshipMod() {
    PatchCall(0x00A1242C, CTemporaryStatView__UpdatePassively_hook); // CWvsContext::SetSkillCooltimeOver
    ATTACH_HOOK(CWvsContext::OnLeaveGame, CWvsContext::OnLeaveGame_hook);
    ATTACH_HOOK(CWvsContext::ClearFieldUI, CWvsContext::ClearFieldUI_hook);
    ATTACH_HOOK(CWvsContext::SetSkillCooltimeOver, CWvsContext::SetSkillCooltimeOver_hook);
}