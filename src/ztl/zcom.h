#pragma once
#include "WzLib/IWzArchive.h"
#include "WzLib/IWzSeekableArchive.h"
#include "WzLib/IWzSerialize.h"
#include "WzLib/IWzProperty.h"
#include "WzLib/IWzNameSpace.h"
#include "WzLib/IWzWritableNameSpace.h"
#include "WzLib/IWzPackage.h"
#include "WzLib/IWzFileSystem.h"
#include "WzLib/IWzResMan.h"
#include "WzLib/IWzUOL.h"
#include "WzLib/IWzCanvas.h"
#include "WzLib/IWzSound.h"
#include "WzLib/IWzShape2D.h"
#include "WzLib/IWzConvex2D.h"
#include "WzLib/IWzVector2D.h"
#include "WzLib/IWzGr2DLayer.h"
#include "WzLib/IWzGr2D.h"

#include "WzLib/IWzGr2DLayer_DX9.h"
#include "WzLib/IWzGr2D_DX9.h"


inline FARPROC* _g_ppfnPCOMAPIs = reinterpret_cast<FARPROC*>(0x00BF0CC0); // changed from FARPROC _g_apfnPCOMAPIs[5];

template<typename T>
void __cdecl PcCreateObject(const wchar_t* sUOL, T& pObj, IUnknown* pUnkOuter) {
    pObj = nullptr;
    if (!_g_ppfnPCOMAPIs[0]) {
        _com_issue_error(CO_E_NOTINITIALIZED);
    }
    _com_util::CheckError(reinterpret_cast<HRESULT (__cdecl*)(const wchar_t*, const GUID*, T&, void*)>(_g_ppfnPCOMAPIs[0])(sUOL, &__uuidof(T), pObj, pUnkOuter));
}

inline void PcSetRootNameSpace(IUnknown* pNameSpace) {
    _com_util::CheckError(reinterpret_cast<HRESULT (__cdecl*)(IUnknown**, int)>(_g_ppfnPCOMAPIs[4])(&pNameSpace, 1));
}