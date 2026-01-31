#include "pch.h"
#include "hook.h"
#include "wvs/util.h"
#include "ztl/ztl.h"


class CWzCanvas : public IWzCanvas {
public:
    typedef HRESULT(__stdcall* raw_Serialize_t)(IWzCanvas*, IWzArchive*);
    inline static raw_Serialize_t raw_Serialize_orig;

    HRESULT __stdcall raw_Serialize_hook(IWzArchive* pArchive) {
        HRESULT hr = raw_Serialize_orig(this, pArchive);
        if (FAILED(hr)) {
            return hr;
        }
        Ztl_variant_t vInlink = this->property->item[L"_inlink"];
        if (V_VT(&vInlink) == VT_BSTR) {
            ZXString<wchar_t> sFilePath(pArchive->absoluteUOL);
            sFilePath.ReleaseBuffer(sFilePath.Find(L".img") + 5);
            sFilePath.Cat(V_BSTR(&vInlink));
            this->property->item[L"_inlink"] = static_cast<const wchar_t*>(sFilePath);
        }
        return hr;
    }
};

void HandleLinkProperty(IWzCanvasPtr pCanvas) {
    // Check for link property
    const wchar_t* asLinkProperty[] = {
            L"_inlink",
            L"_outlink",
            L"source",
    };
    size_t nLinkProperty = sizeof(asLinkProperty) / sizeof(asLinkProperty[0]);
    for (size_t i = 0; i < nLinkProperty; ++i) {
        Ztl_variant_t vLink = pCanvas->property->item[asLinkProperty[i]];
        if (V_VT(&vLink) == VT_BSTR) {
            // Get source canvas
            IWzCanvasPtr pSourceCanvas = get_rm()->GetObjectA(V_BSTR(&vLink)).GetUnknown();
            int32_t nWidth, nHeight, nFormat, nMagLevel;
            pSourceCanvas->GetSnapshot(&nWidth, &nHeight, nullptr, nullptr, (CANVAS_PIXFORMAT*)&nFormat, &nMagLevel);

            // Create target canvas
            pCanvas->Create(nWidth, nHeight, nMagLevel, nFormat);
            pCanvas->AddRawCanvas(0, 0, pSourceCanvas->rawCanvas[0][0]);

            // Set target origin
            IWzVector2DPtr pOrigin = pCanvas->property->item[L"origin"].GetUnknown();
            pCanvas->cx = pOrigin->x;
            pCanvas->cy = pOrigin->y;
            break;
        }
    }
}

static auto get_unknown_orig = reinterpret_cast<IUnknownPtr*(__cdecl*)(IUnknownPtr*, Ztl_variant_t*)>(0x00414ADA);
IUnknownPtr* __cdecl get_unknown_hook(IUnknownPtr* result, Ztl_variant_t* v) {
    get_unknown_orig(result, v);
    IWzCanvasPtr pCanvas;
    if (SUCCEEDED(result->QueryInterface(__uuidof(IWzCanvas), &pCanvas))) {
        HandleLinkProperty(pCanvas);
    }
    return result;
}

static auto Ztl_variant_t__GetUnknown = reinterpret_cast<IUnknown*(__thiscall*)(Ztl_variant_t*, bool, bool)>(0x004032B2);
IUnknown* __fastcall Ztl_variant_t__GetUnknown_hook(Ztl_variant_t* pThis, void* _EDX, bool fAddRef, bool fTryChangeType) {
    IUnknownPtr result = pThis->GetUnknown(fAddRef, fTryChangeType);
    IWzCanvasPtr pCanvas;
    if (SUCCEEDED(result.QueryInterface(__uuidof(IWzCanvas), &pCanvas))) {
        HandleLinkProperty(pCanvas);
    }
    return result;
}


void AttachClientInlink() {
    CWzCanvas::raw_Serialize_orig = reinterpret_cast<CWzCanvas::raw_Serialize_t>(GetAddressByPattern("CANVAS.DLL", "B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 6C"));
    ATTACH_HOOK(CWzCanvas::raw_Serialize_orig, CWzCanvas::raw_Serialize_hook);
    ATTACH_HOOK(get_unknown_orig, get_unknown_hook);
    ATTACH_HOOK(Ztl_variant_t__GetUnknown, Ztl_variant_t__GetUnknown_hook); // for cases where nexon uses this instead of get_unknown
}