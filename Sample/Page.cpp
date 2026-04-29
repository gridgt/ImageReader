#include "pch.h"
#include "Page.h"
#include "Window.h"
#include "Env.h"
#include <OcrLiteCApi.h>

Page::Page(Window* win, ICoreWebView2* webview) :win{ win }, webview{ webview }
{
    ComPtr<ICoreWebView2Settings> settings;
    webview->get_Settings(&settings);
    settings->put_AreDefaultContextMenusEnabled(false);
    settings->put_IsZoomControlEnabled(false);
    settings->put_IsStatusBarEnabled(false);

    webview->AddWebResourceRequestedFilter(L"https://app.localhost/*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
    auto resRequestedCB = Callback<ICoreWebView2WebResourceRequestedEventHandler>(this, &Page::onRequest);
    webview->add_WebResourceRequested(resRequestedCB.Get(), nullptr);

    ComPtr<ICoreWebView2_2> webview2;
    this->webview.As(&webview2);
    auto domLoadedCB = Callback<ICoreWebView2DOMContentLoadedEventHandler>(this, &Page::onDomLoaded);
    webview2->add_DOMContentLoaded(domLoadedCB.Get(), nullptr);


    auto msgReceivedCB = Callback<ICoreWebView2WebMessageReceivedEventHandler>(this, &Page::onMsgReceived);
    webview->add_WebMessageReceived(msgReceivedCB.Get(), nullptr);

    
}

Page::~Page()
{
}
void Page::navigate(const std::wstring& url)
{
    webview->Navigate(url.data());
}

HRESULT Page::onDomLoaded(ICoreWebView2* sender, ICoreWebView2DOMContentLoadedEventArgs* args)
{
    //ComPtr<ICoreWebView2_6> webview6;
    //this->webview.As(&webview6);
    //webview6->OpenTaskManagerWindow();
    return S_OK;
}

HRESULT Page::onMsgReceived(ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args)
{
    PWSTR jsonRaw;
    auto hr = args->get_WebMessageAsJson(&jsonRaw);
    if (FAILED(hr)) return S_OK;
    JsonObject param = JsonObject::Parse(jsonRaw);
    CoTaskMemFree(jsonRaw);
    auto method = param.GetNamedString(L"method");
    JsonObject result;
    if (param.HasKey(L"id")) {
        result.SetNamedValue(L"id", JsonValue::CreateStringValue(param.GetNamedString(L"id")));
    }    
    if (method == L"selectImageFile") {
        openOneFile();
    }
    auto resultStr = result.Stringify();
    webview->PostWebMessageAsJson(resultStr.data());
    return S_OK;
}


HRESULT Page::onRequest(ICoreWebView2* webview, ICoreWebView2WebResourceRequestedEventArgs* args)
{
    ComPtr<ICoreWebView2WebResourceRequest> request;
    args->get_Request(&request);
    LPWSTR rawUri = nullptr;
    request->get_Uri(&rawUri);
    std::wstring url(rawUri);
    CoTaskMemFree(rawUri);
    size_t slashPos = url.find_last_of(L'/');
    size_t queryPos = url.find(L'?');
    size_t start = (slashPos != std::wstring::npos) ? slashPos + 1 : 0;
    size_t end = (queryPos != std::wstring::npos) ? queryPos : url.length();
    if (start >= end || start >= url.length()) return S_OK;
    std::wstring resName = url.substr(start, end - start);
    HRSRC hRes = FindResource(NULL, resName.data(), RT_RCDATA);
    if (!hRes) return S_OK;
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return S_OK;
    void* pData = LockResource(hData);
    DWORD size = SizeofResource(NULL, hRes);
    ComPtr<IStream> stream = SHCreateMemStream((const BYTE*)pData, size);
    auto ct = getContentType(resName);
    ComPtr<ICoreWebView2WebResourceResponse> response;
    Env::getWebViewEnv()->CreateWebResourceResponse(stream.Get(), 200, L"OK", ct.data(), &response);
    args->put_Response(response.Get());
    return S_OK;
}

std::wstring Page::getContentType(const std::wstring& fileName)
{
    static const std::unordered_map<std::string, std::wstring> mimeTypes = {
        {".html", L"Content-Type: text/html"},
        {".htm",  L"Content-Type: text/html"},
        {".js",   L"Content-Type: application/javascript"},
        {".css",  L"Content-Type: text/css"},
        {".json", L"Content-Type: application/json"},
        {".png",  L"Content-Type: image/png"},
        {".jpg",  L"Content-Type: image/jpeg"},
        {".jpeg", L"Content-Type: image/jpeg"},
        {".gif",  L"Content-Type: image/gif"},
        {".svg",  L"Content-Type: image/svg+xml"},
        {".ico",  L"Content-Type: image/x-icon"},
        {".woff", L"Content-Type: font/woff"},
        {".woff2",L"Content-Type: font/woff2"},
        {".ttf",  L"Content-Type: font/ttf"},
        {".eot",  L"Content-Type: application/vnd.ms-fontobject"},
        {".txt",  L"Content-Type: text/plain"},
        {".wasm", L"Content-Type: application/wasm"},
        {".mp3",  L"Content-Type: audio/mpeg"},
        {".mp4",  L"Content-Type: video/mp4"}
    };
    std::filesystem::path path(fileName);
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    auto it = mimeTypes.find(ext);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return L"Content-Type: application/octet-stream";
}

std::string convertToStr(const std::wstring& wstr)
{
    const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

void Page::openOneFile()
{
    Pickers::FileOpenPicker picker;
    auto init = picker.as<IInitializeWithWindow>();
    init->Initialize(win->hwnd);
    picker.SuggestedStartLocation(Pickers::PickerLocationId::Desktop);
    picker.SettingsIdentifier(L"Sample");
    auto filters = picker.FileTypeFilter();
    filters.Append(L".png");
    filters.Append(L".jpg");
    filters.Append(L".jpeg");
    picker.CommitButtonText(L"选择文件");
    auto fileOp = picker.PickSingleFileAsync();
    fileOp.Completed([this](auto& sender, auto status) {
        StorageFile file = sender.GetResults();
        if (file) {
            std::filesystem::path path(std::wstring{ file.Path() });
            std::wstring dirPath = path.parent_path().wstring();
            std::wstring fileName = path.filename().wstring();
            auto dirStr = convertToStr(dirPath);
            auto nameStr = convertToStr(fileName);

            Env::getDispatcherQueue().TryEnqueue([this, dirStr,nameStr]()
                {

                    auto handle = Env::getOcrHandle();
                    OCR_PARAM param = { 0 };
                    OCR_BOOL bRet = OcrDetect(handle,"C:/Users/liulun/Desktop/img/", "1.jpg", &param);
                    if (bRet) {
                        int nLen = OcrGetLen(handle);
                        if (nLen > 0) {
                            char* szInfo = (char*)malloc(nLen);
                            if (szInfo) {
                                if (OcrGetResult(handle, szInfo, nLen)) {
                                    printf("%s", szInfo);
                                }
                                free(szInfo);
                            }
                        }
                    }

                    JsonObject result;
                    result.SetNamedValue(L"eventName", JsonValue::CreateStringValue(L"imageFileSelected"));
                    //result.SetNamedValue(L"filePath", JsonValue::CreateStringValue(str.data()));
                    winrt::hstring jsonString = result.Stringify();
                    this->webview->PostWebMessageAsJson(jsonString.data());
                });
        }
    });
}