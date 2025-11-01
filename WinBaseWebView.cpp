#include "WinBase.h"
#include "Message.h"

void WinBase::initView(ICoreWebView2Environment* env)
{
    auto ctrlReadyInstance = Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this](HRESULT result, ICoreWebView2Controller* ctrl) {
        if (FAILED(result))
        {
            MessageBox(NULL, L"Failed to create webview2 controller", L"Error", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }
        webviewCtrl = ctrl;
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
        addRequestFilter();
        addMsgReceiver();
        addDomLoader();
        onViewReady();

        RECT bounds;
        GetClientRect(hwnd, &bounds);
        webviewCtrl->put_Bounds(bounds);
        return S_OK;
    });
    env->CreateCoreWebView2Controller(hwnd, ctrlReadyInstance.Get());
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
    if (resName[0]==L'$'&&resName[1] == L'$') {
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
    auto hd = L"Content-Type: " + ct;
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
            getFiles(arg, msg);
            msg->route();
            return S_OK;
        }
    );
    webview->add_WebMessageReceived(msgReceivedCB.Get(), &msgReceivedToken);
}
void WinBase::addDomLoader()
{
    auto domLoadedCB = Callback<ICoreWebView2DOMContentLoadedEventHandler>([this](auto wv,auto arg) {
        //wv->OpenDevToolsWindow();
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

void WinBase::getFiles(ICoreWebView2WebMessageReceivedEventArgs* args, Message* msg)
{
    ComPtr<ICoreWebView2WebMessageReceivedEventArgs2> args2;
    auto hr = args->QueryInterface(IID_PPV_ARGS(&args2));
    if (FAILED(hr)) return;
    ComPtr<ICoreWebView2ObjectCollectionView> additionalObjects;
    hr = args2->get_AdditionalObjects(&additionalObjects);
    if (FAILED(hr)|| !additionalObjects.Get()) return;
    UINT32 count = 0;
    additionalObjects->get_Count(&count);

    JsonArray arr;
    for (size_t i = 0; i < count; i++)
    {
        ComPtr<ICoreWebView2File> file;
        additionalObjects->GetValueAtIndex(i, &file);
        PWSTR path;
        hr = file->get_Path(&path);
        arr.Append(JsonValue::CreateStringValue(path));
        CoTaskMemFree(path);
    }
    msg->result.SetNamedValue(L"$files", arr);
}