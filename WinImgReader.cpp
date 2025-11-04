#include <dwmapi.h>
#include <ocrAPI.h>
#include "WinImgReader.h"
#include "Message.h"
#include "Environment.h"

std::unique_ptr<WinImgReader> instance;

WinImgReader::WinImgReader()
{
    this->ocrTask = initOCR();
    initPosSize();
    createWindow();
    show();
}

WinImgReader::~WinImgReader()
{
    ocrUninit();
}

void WinImgReader::init()
{
    instance = std::make_unique<WinImgReader>();
}

WinImgReader* WinImgReader::get()
{
    return instance.get();
}

winrt::Windows::Foundation::IAsyncAction WinImgReader::initOCR()
{
    co_await winrt::resume_background();
    ocrInit(true);
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


std::string hstring_to_ansi(const winrt::hstring& hstr) {
    if (hstr.empty()) {
        return {};
    }

    // 1. 获取 wchar_t* 视图
    auto wstr = static_cast<std::wstring_view>(hstr);

    // 2. 计算所需 ANSI 缓冲区大小（CP_ACP = 当前系统 ANSI 代码页）
    int size = WideCharToMultiByte(CP_ACP, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {}; // 转换失败
    }

    // 3. 分配并转换
    std::string ansi(size, '\0');
    WideCharToMultiByte(CP_ACP, 0, wstr.data(), (int)wstr.size(), ansi.data(), size, nullptr, nullptr);

    return ansi;
}

winrt::Windows::Foundation::IAsyncAction WinImgReader::readImg(Message* msg)
{
    if (ocrTask.Status() != winrt::Windows::Foundation::AsyncStatus::Completed)
    {
        co_await ocrTask;
    }
    
    co_await winrt::resume_background();
    auto arr = msg->result.GetNamedArray(L"$files");
    imgPath = arr.GetStringAt(0);
	auto imgPathStr = hstring_to_ansi(imgPath);
    char* resultStr{nullptr};
    ocrRecognize(imgPathStr.data(), &resultStr);
    auto result = winrt::to_hstring(resultStr);
    free(resultStr);
	JsonObject resultObj = JsonObject::Parse(result);
    msg->result.SetNamedValue(L"lines", resultObj.GetNamedArray(L"rec_texts"));
    msg->result.SetNamedValue(L"polys", resultObj.GetNamedArray(L"rec_polys"));
    co_await winrt::resume_foreground(Environment::get()->dq);
    msg->resolve();
}



