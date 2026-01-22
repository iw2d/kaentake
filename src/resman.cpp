#include "pch.h"
#include "hook.h"
#include "debug.h"
#include "wvs/wvsapp.h"
#include "wvs/util.h"
#include <algorithm>
#include <vector>
#include <tuple>

static IWzNameSpacePtr g_pCustomNameSpace;
static std::vector<Ztl_bstr_t> g_vecOverrides;

class CWzProperty : public IWzProperty {
public:
    typedef HRESULT(__stdcall* raw_Serialize_t)(IWzProperty*, IWzArchive*);
    inline static raw_Serialize_t raw_Serialize_orig;

    HRESULT __stdcall raw_Serialize_hook(IWzArchive* pArchive) {
        HRESULT hr = raw_Serialize_orig(this, pArchive);
        if (FAILED(hr)) {
            return hr;
        }
        if (!std::binary_search(g_vecOverrides.begin(), g_vecOverrides.end(), pArchive->absoluteUOL)) {
            return hr;
        }
        IWzPropertyPtr pProperty = get_rm()->GetObjectA(L"Custom/" + pArchive->absoluteUOL).GetUnknown();
        IEnumVARIANTPtr pEnum = pProperty->_NewEnum;
        while (true) {
            Ztl_variant_t vNext;
            ULONG uCeltFetched;
            if (FAILED(pEnum->Next(1, &vNext, &uCeltFetched)) || uCeltFetched == 0) {
                break;
            }
            Ztl_bstr_t sNext = V_BSTR(&vNext);
            IUnknownPtr pUnk = pProperty->item[sNext].GetUnknown();
            IWzPropertyPtr pSub;
            if (!pUnk || FAILED(pUnk->QueryInterface(&pSub))) {
                this->Add(sNext, pProperty->item[sNext], false);
            }
        }
        return S_OK;
    }
};


void CWvsApp::InitializeResMan_hook() {
    DEBUG_MESSAGE("CWvsApp::InitializeResMan");
    CWvsApp::InitializeResMan(this);

    // add custom namespace to root
    IWzWritableNameSpacePtr pWritableRoot;
    if (FAILED(get_root()->QueryInterface(&pWritableRoot))) {
        ErrorMessage("Failed to cast root namespace");
        return;
    }
    IWzNameSpacePtr pNameSpace;
    PcCreateObject<IWzNameSpacePtr>(L"NameSpace", pNameSpace, nullptr);
    Ztl_variant_t vResult;
    pWritableRoot->AddObject(L"Custom", static_cast<IUnknown*>(pNameSpace), &vResult);
    g_pCustomNameSpace = vResult.GetUnknown();

    // load Custom.wz from file system
    IWzFileSystemPtr fs;
    PcCreateObject<IWzFileSystemPtr>(L"NameSpace#FileSystem", fs, nullptr);
    char sStartPath[MAX_PATH];
    GetModuleFileNameA(nullptr, sStartPath, MAX_PATH);
    Dir_BackSlashToSlash(sStartPath);
    Dir_upDir(sStartPath);
    fs->Init(sStartPath);

    IWzPackagePtr pPackage;
    PcCreateObject<IWzPackagePtr>(L"NameSpace#Package", pPackage, nullptr);
    IWzSeekableArchivePtr pArchive = fs->item[L"Custom.wz"].GetUnknown();
    pPackage->Init(L"83", L"Custom", pArchive);
    g_pCustomNameSpace->Mount(L"/", pPackage, 1);

    // iterate custom namespace
    std::vector<std::tuple<Ztl_bstr_t, IEnumVARIANTPtr>> stack;
    stack.emplace_back(L"", g_pCustomNameSpace->_NewEnum);
    while (!stack.empty()) {
        auto [sPath, pEnum] = stack.back();
        stack.pop_back();

        while (true) {
            Ztl_variant_t vNext;
            ULONG uCeltFetched;
            if (FAILED(pEnum->Next(1, &vNext, &uCeltFetched)) || uCeltFetched == 0) {
                break;
            }
            Ztl_bstr_t sUOL = (sPath.length() > 0 ? sPath + L"/" : L"") + V_BSTR(&vNext);
            Ztl_variant_t vObj = get_rm()->GetObjectA(L"Custom/" + sUOL);
            IWzNameSpacePtr pSub;
            if (SUCCEEDED(vObj.GetUnknown()->QueryInterface(&pSub))) {
                stack.emplace_back(sUOL, pSub->_NewEnum);
                continue;
            }
            IWzPropertyPtr pProp;
            if (SUCCEEDED(vObj.GetUnknown()->QueryInterface(&pProp))) {
                g_vecOverrides.push_back(sUOL);
                stack.emplace_back(sUOL, pProp->_NewEnum);
                continue;
            }
        }
    }
    std::sort(g_vecOverrides.begin(), g_vecOverrides.end()); // uses operator<

    // PCOM.dll - override objects by hooking CWzProperty::raw_Serialize
    CWzProperty::raw_Serialize_orig = static_cast<CWzProperty::raw_Serialize_t>(GetAddressByPattern("PCOM.dll", "B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 68"));
    ATTACH_HOOK(CWzProperty::raw_Serialize_orig, CWzProperty::raw_Serialize_hook);
}

void CWvsApp::CleanUp_hook() {
    CWvsApp::CleanUp(this);
    g_pCustomNameSpace = nullptr;
}


void AttachResManMod() {
    ATTACH_HOOK(CWvsApp::InitializeResMan, CWvsApp::InitializeResMan_hook);
    ATTACH_HOOK(CWvsApp::CleanUp, CWvsApp::CleanUp_hook);
}