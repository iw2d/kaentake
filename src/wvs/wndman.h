#pragma once
#include "hook.h"
#include "wvs/wnd.h"
#include "wvs/util.h"
#include "ztl/ztl.h"
#include <windows.h>


class CWndMan : public CWnd, public TSingleton<CWndMan, 0x00BEC20C> {
public:
    inline static ZList<CWnd*>& ms_lpWindow = *reinterpret_cast<ZList<CWnd*>*>(0x00BF1648);
    inline static IWzVector2DPtr ms_pOrgWindowEx[UIOrigin::Origin_NUM];

    MEMBER_AT(IWzVector2DPtr, 0xDC, m_pOrgWindow)

    // bypass.cpp
    MEMBER_HOOK(int, 0x009E7D77, TranslateMessage, UINT& msg, WPARAM& wParam, LPARAM& lParam, LRESULT* plResult);

    // resolution.cpp
    MEMBER_HOOK(void, 0x009E2C42, Constructor, HWND hWnd)
    MEMBER_HOOK(void, 0x009E3026, Destructor)
    MEMBER_HOOK(IWzVector2DPtr*, 0x0048BBA5, GetOrgWindow, IWzVector2DPtr* result)

    inline IWzVector2DPtr& GetOrgWindowEx(CWnd::UIOrigin org) {
        return ms_pOrgWindowEx[org];
    }
    inline void ResetOrgWindow() {
        m_pOrgWindow->origin = static_cast<IUnknown*>(get_gr()->center);
        m_pOrgWindow->RelMove(-(get_screen_width() / 2), -(get_screen_height() / 2) - get_adjust_cy());
        for (int i = 0; i < UIOrigin::Origin_NUM; ++i) {
            int nX = -(get_screen_width() / 2);
            if (i % 3 == 1) {
                nX += (get_screen_width() - 800) / 2;
            } else if (i % 3 == 2) {
                nX += (get_screen_width() - 800);
            }
            int nY = -(get_screen_height() / 2) - get_adjust_cy();
            if (i / 3 == 1) {
                nY += (get_screen_height() - 600) / 2;
            } else if (i / 3 == 2) {
                nY += (get_screen_height() - 600);
            }
            ms_pOrgWindowEx[i]->origin = static_cast<IUnknown*>(get_gr()->center);
            ms_pOrgWindowEx[i]->RelMove(nX, nY);
        }
    }
};