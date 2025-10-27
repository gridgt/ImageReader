#include <filesystem>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>

#include "WindowBase.h"
#include "WebView.h"
#include "Message.h"
#include "Environment.h"

WebView::WebView(WindowBase* parentWindow, ICoreWebView2_22* webview) : 
    parentWindow{ parentWindow }, webview{webview}
{
    addRequestFilter();

    EventRegistrationToken msgReceivedToken;
    auto msgReceivedCB = Callback<ICoreWebView2WebMessageReceivedEventHandler>(this, &WebView::msgReceived);
    webview->add_WebMessageReceived(msgReceivedCB.Get(), &msgReceivedToken);

    auto domLoadedCB = Callback<ICoreWebView2DOMContentLoadedEventHandler>(this, &WebView::domLoaded);
    EventRegistrationToken domLoadedToken;
    webview->add_DOMContentLoaded(domLoadedCB.Get(), &domLoadedToken);

    webview->Navigate(L"https://app.localhost/index.html");
}

WebView::~WebView()
{
}

HRESULT WebView::msgReceived(ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args)
{
    PWSTR jsonRaw;
    auto hr = args->get_WebMessageAsJson(&jsonRaw);
    if (FAILED(hr)) {
        throw std::invalid_argument("WebView::getParam get_WebMessageAsJson err");
    }
    JsonObject param = JsonObject::Parse(jsonRaw);
    CoTaskMemFree(jsonRaw);
    auto msg = new Message(std::move(param), webview, this->parentWindow);
    msg->route();
    return S_OK;
}

HRESULT WebView::domLoaded(ICoreWebView2* webview, ICoreWebView2DOMContentLoadedEventArgs* args)
{
    webview->OpenDevToolsWindow();
    return S_OK;
}

HRESULT WebView::resRequested(ICoreWebView2* wv, ICoreWebView2WebResourceRequestedEventArgs* args)
{
    ComPtr<ICoreWebView2WebResourceRequest> request;
    args->get_Request(&request);
    LPWSTR rawUri = nullptr;
    request->get_Uri(&rawUri);
    std::wstring url(rawUri);
    CoTaskMemFree(rawUri);

    size_t pos0 = url.find(L".localhost/") + wcslen(L".localhost/");
    size_t pos1 = url.find(L"?", pos0);
    std::wstring resName = url.substr(pos0, pos1 == std::wstring::npos ? std::wstring::npos : pos1 - pos0);

    ComPtr<ICoreWebView2WebResourceResponse> response;
    HRSRC hRes = FindResource(NULL, resName.data(), RT_RCDATA);
    if (!hRes) {
        args->put_Response(nullptr);
        return S_OK;
    }
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        args->put_Response(nullptr);
        return S_OK;
    }
    void* pData = LockResource(hData);
    DWORD size = SizeofResource(NULL, hRes);
    ComPtr<IStream> stream = SHCreateMemStream((const BYTE*)pData, size);
    auto ct = getContentType(resName);
    auto hd = std::format(L"Content-Type: {}", ct.data());

    ComPtr<ICoreWebView2Environment> env;
    webview->get_Environment(&env);
    env->CreateWebResourceResponse(stream.Get(), 200, L"OK", hd.data(), &response);
    args->put_Response(response.Get());
    return S_OK;
}


void WebView::addRequestFilter()
{
    webview->AddWebResourceRequestedFilterWithRequestSourceKinds(L"*", 
        COREWEBVIEW2_WEB_RESOURCE_CONTEXT_DOCUMENT, 
        COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_DOCUMENT);// 过滤 HTML    
    webview->AddWebResourceRequestedFilterWithRequestSourceKinds(L"*",
        COREWEBVIEW2_WEB_RESOURCE_CONTEXT_STYLESHEET, 
        COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_DOCUMENT);// 过滤 CSS    
    webview->AddWebResourceRequestedFilterWithRequestSourceKinds(L"*", 
        COREWEBVIEW2_WEB_RESOURCE_CONTEXT_SCRIPT,
        COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_DOCUMENT);// 过滤 JS    
    webview->AddWebResourceRequestedFilterWithRequestSourceKinds(L"*",
        COREWEBVIEW2_WEB_RESOURCE_CONTEXT_IMAGE,
        COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_DOCUMENT);// 过滤 图片 
    webview->AddWebResourceRequestedFilterWithRequestSourceKinds(L"*", 
        COREWEBVIEW2_WEB_RESOURCE_CONTEXT_MEDIA,
        COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_DOCUMENT);// 过滤 媒体    
    webview->AddWebResourceRequestedFilterWithRequestSourceKinds(L"*", 
        COREWEBVIEW2_WEB_RESOURCE_CONTEXT_FONT,
        COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_DOCUMENT);// 过滤 字体
    auto resRequestedCB = Callback<ICoreWebView2WebResourceRequestedEventHandler>(this, &WebView::resRequested);
    EventRegistrationToken resRequestedToken;
    auto hr = webview->add_WebResourceRequested(resRequestedCB.Get(), &resRequestedToken);
    //webview->remove_WebResourceRequested(resRequestedToken);
}

std::wstring WebView::getContentType(const std::wstring& fileName)
{
    static const std::unordered_map<std::string, std::wstring> mimeTypes = {
            {".html", L"text/html"},
            {".htm",  L"text/html"},
            {".js",   L"application/javascript"},
            {".css",  L"text/css"},
            {".json", L"application/json"},
            {".png",  L"image/png"},
            {".jpg",  L"image/jpeg"},
            {".jpeg", L"image/jpeg"},
            {".gif",  L"image/gif"},
            {".svg",  L"image/svg+xml"},
            {".ico",  L"image/x-icon"},
            {".woff", L"font/woff"},
            {".woff2",L"font/woff2"},
            {".ttf",  L"font/ttf"},
            {".eot",  L"application/vnd.ms-fontobject"},
            {".txt",  L"text/plain"},
            {".wasm",  L"application/wasm"},
            {".mp3",  L"audio/mpeg"},
            {".mp4",  L"video/mp4"}
    };
    std::filesystem::path path(fileName);
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    auto it = mimeTypes.find(ext);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return L"application/octet-stream";
}