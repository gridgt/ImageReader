#include "WinBase.h"
#include "Message.h"

void WinBase::createCompCtrl()
{
    DispatcherQueueOptions options{
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_ASTA
    };
    static winrt::Windows::System::DispatcherQueueController dispatchCtrl{ nullptr };
    if (!dispatchCtrl) {
        CreateDispatcherQueueController(options,
            reinterpret_cast<ABI::Windows::System::IDispatcherQueueController**>(winrt::put_abi(dispatchCtrl)));
    }    
    compositor = winrt::Windows::UI::Composition::Compositor();
}

void WinBase::initView(ICoreWebView2Environment* env)
{
    auto ready = Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
        this, &WinBase::ctrlReady);
    ICoreWebView2Environment3* env3;
    env->QueryInterface(IID_PPV_ARGS(&env3));
    env3->CreateCoreWebView2CompositionController(hwnd, ready.Get());
}

HRESULT WinBase::ctrlReady(HRESULT result, ICoreWebView2CompositionController* ctrlComp)
{
    if (FAILED(result))
    {
        MessageBox(NULL, L"Failed to create webview2 controller", L"Error", MB_OK | MB_ICONERROR);
        ExitProcess(-1);
    }

    this->ctrlComp = ctrlComp;
    this->ctrlComp.As(&this->webviewCtrl);
    ComPtr<ICoreWebView2> webview;
    webviewCtrl->get_CoreWebView2(&webview);
    auto hr = webview.As(&this->webview);
    if (FAILED(hr)) {
        auto result = MessageBox(nullptr, L"WebView2系统组件版本过低，请安装新版本",
            L"系统提示", MB_OKCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON1);
        if (result == IDOK) {
            ShellExecute(0, 0, L"https://go.microsoft.com/fwlink/p/?LinkId=2124703", 0, 0, SW_SHOW);
        }
        ExitProcess(-1);
        return S_OK;
    }
    bindCompCtrlToHwnd();
    addRequestFilter();
    addMsgReceiver();
    addDomLoader();
    onViewReady();
    return S_OK;
}

void WinBase::bindCompCtrlToHwnd()
{
    auto interop = compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();
    interop->CreateDesktopWindowTarget(
        hwnd, false,
        reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget**>(winrt::put_abi(winTarget))
    );
    rootVisual = compositor.CreateContainerVisual();
    rootVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
    rootVisual.Offset({ 0, 0, 0 });
    winTarget.Root(rootVisual);
    webviewVisual = compositor.CreateContainerVisual();
    rootVisual.Children().InsertAtTop(webviewVisual);
    this->ctrlComp->put_RootVisualTarget(webviewVisual.as<IUnknown>().get());
    RECT bounds;
    GetClientRect(hwnd, &bounds);
    webviewCtrl->put_Bounds(bounds);
}

HRESULT WinBase::resRequested(ICoreWebView2* wv, ICoreWebView2WebResourceRequestedEventArgs* args)
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
    
    ComPtr<IStream> stream;
    if (resName.starts_with(L"$$")) {
        stream = procLocalRes(resName);
    }
    else {
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
        stream = SHCreateMemStream((const BYTE*)pData, size);
    }    
    ComPtr<ICoreWebView2WebResourceResponse> response;
    auto ct = getContentType(resName);
    auto hd = std::format(L"Content-Type: {}", ct.data());
    ComPtr<ICoreWebView2Environment> env;
    webview->get_Environment(&env);
    env->CreateWebResourceResponse(stream.Get(), 200, L"OK", hd.data(), &response);
    args->put_Response(response.Get());
    return S_OK;
}


void WinBase::addRequestFilter()
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
    auto resRequestedCB = Callback<ICoreWebView2WebResourceRequestedEventHandler>(this, &WinBase::resRequested);
    EventRegistrationToken resRequestedToken;
    auto hr = webview->add_WebResourceRequested(resRequestedCB.Get(), &resRequestedToken);
    //webview->remove_WebResourceRequested(resRequestedToken);
}

std::wstring WinBase::getContentType(const std::wstring& fileName)
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

void WinBase::addMsgReceiver()
{
    EventRegistrationToken msgReceivedToken;
    auto msgReceivedCB = Callback<ICoreWebView2WebMessageReceivedEventHandler>(
        [this](auto wv, auto arg) {
            auto param = getParam(arg);
            auto msg = new Message(std::move(param), this);
            msg->route();
            return S_OK;
        }
    );
    webview->add_WebMessageReceived(msgReceivedCB.Get(), &msgReceivedToken);
}
void WinBase::addDomLoader()
{
    auto domLoadedCB = Callback<ICoreWebView2DOMContentLoadedEventHandler>([this](auto wv,auto arg) {
        wv->OpenDevToolsWindow();
        return S_OK;
    });
    EventRegistrationToken domLoadedToken;
    webview->add_DOMContentLoaded(domLoadedCB.Get(), &domLoadedToken);
}

JsonObject WinBase::getParam(ICoreWebView2WebMessageReceivedEventArgs* args)
{
    PWSTR jsonRaw;
    auto hr = args->get_WebMessageAsJson(&jsonRaw);
    if (FAILED(hr)) {
        throw std::invalid_argument("WebView::getParam get_WebMessageAsJson err");
    }
    JsonObject param = JsonObject::Parse(jsonRaw);
    CoTaskMemFree(jsonRaw);
    return param;
}