#include <dwmapi.h>
#include "WinImgReader.h"
#include "Message.h"
#include "Environment.h"

std::unique_ptr<WinImgReader> instance;

WinImgReader::WinImgReader()
{
    initPosSize();
    createWindow();
    show();
}

WinImgReader::~WinImgReader()
{
}

void WinImgReader::init()
{
    instance = std::make_unique<WinImgReader>();
}

WinImgReader* WinImgReader::get()
{
    return instance.get();
}

void WinImgReader::initPosSize()
{
    w = 1000;
    h = 800;
    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMon, &mi);
    RECT work = mi.rcWork;
    int width = work.right - work.left;
    int height = work.bottom - work.top;
    x = work.left + (width - w) / 2;
    y = work.top + (height - h) / 2;
}

void WinImgReader::onViewReady()
{
    webview->Navigate(L"https://app.localhost/viewer.html");
}

LRESULT WinImgReader::procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_GETMINMAXINFO) {
        setMinMaxInfo((LPMINMAXINFO)lParam);
        return 0;
    }
    else if (msg == WM_SIZE) {
        onSize(wParam);
        return 0;
    }
    else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return WinBase::procNativeMsg(msg, wParam, lParam);
}

void WinImgReader::procProcMsg(Message* msg)
{
    auto methodName = msg->param.GetNamedString(L"$methodName");
    if (methodName == L"readImg") {
        readImg(msg);
    }
}

void WinImgReader::onSize(UINT param)
{
    if (param == SIZE_MAXIMIZED) {
        if (eventTargets.contains(L"win_maximize")) {
            eventTargets[L"win_maximize"]->resolve();
        }
    }
    else if (param == SIZE_RESTORED) {
        if (eventTargets.contains(L"win_restore")) {
            eventTargets[L"win_restore"]->resolve();
        }
    }
    if (webviewCtrl)
    {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        webviewCtrl->put_Bounds(bounds);
    }
}

void WinImgReader::setMinMaxInfo(LPMINMAXINFO lpMMI)
{
    MONITORINFO mi = { sizeof(mi) };
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfo(hMonitor, &mi)) {
        lpMMI->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
        lpMMI->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
        lpMMI->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
        lpMMI->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
    }
    lpMMI->ptMinTrackSize.x = 800;
    lpMMI->ptMinTrackSize.y = 600;
}

ComPtr<IStream> WinImgReader::procLocalRes(std::wstring& resName)
{
    auto ext = std::filesystem::path(imgPath.data()).extension().wstring();
    resName = L"$$img." + ext;
    ComPtr<IStream> stream;
    HRESULT hr = SHCreateStreamOnFile(imgPath.data(), STGM_READ, stream.GetAddressOf());
    return stream;
}

winrt::Windows::Foundation::IAsyncAction WinImgReader::readImg(Message* msg)
{
    JsonArray lineArr;
    JsonArray wordArr;
    auto arr = msg->result.GetNamedArray(L"$files");
    imgPath = arr.GetStringAt(0);
    auto file = co_await winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(imgPath.data());
    auto stream = co_await file.OpenAsync(winrt::Windows::Storage::FileAccessMode::Read);
    auto decoder = co_await winrt::Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(stream);
    auto softwareBitmap = co_await decoder.GetSoftwareBitmapAsync();
    //// 2. 转为灰度（OcrEngine 要求 B8G8R8A8 或 Gray8）
    //if (softwareBitmap.BitmapPixelFormat() != BitmapPixelFormat::Gray8)
    //{
    //    softwareBitmap = SoftwareBitmap::Convert(softwareBitmap, BitmapPixelFormat::Gray8);
    //}
    winrt::Windows::Media::Ocr::OcrEngine ocrEngine = winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();
    winrt::Windows::Media::Ocr::OcrResult result = co_await ocrEngine.RecognizeAsync(softwareBitmap);
    std::wstring text;
    int lineIndex{ 0 };
    for (auto const& line : result.Lines())
    {
        JsonObject lineObj;
        auto text = line.Text();
        lineObj.SetNamedValue(L"text", JsonValue::CreateStringValue(line.Text()));
        lineArr.Append(lineObj);
        auto count = line.Words().Size();
        for (auto const& word : line.Words())
        {
            JsonObject wordObj;
            winrt::hstring text = word.Text();
            wordObj.SetNamedValue(L"text", JsonValue::CreateStringValue(text));
            winrt::Windows::Foundation::Rect wordRect = word.BoundingRect();
            float wx1{ wordRect.X }, wy1{wordRect.Y}, wx2{ wordRect.Width }, wy2{ wordRect.Height };
            wordObj.SetNamedValue(L"x1", JsonValue::CreateNumberValue(wx1));
            wordObj.SetNamedValue(L"y1", JsonValue::CreateNumberValue(wy1));
            wordObj.SetNamedValue(L"x2", JsonValue::CreateNumberValue(wx2));
            wordObj.SetNamedValue(L"y2", JsonValue::CreateNumberValue(wy2));
            wordObj.SetNamedValue(L"lineIndex", JsonValue::CreateNumberValue(lineIndex));
        }
        lineIndex += 1;
    }
    msg->result.SetNamedValue(L"lines", lineArr);
    msg->result.SetNamedValue(L"words", wordArr);
    co_await winrt::resume_foreground(Environment::get()->dq);
    msg->resolve();
}



