#include <dwmapi.h>
#include <ocrAPI.h>
#include "WinImgReader.h"
#include "Message.h"
#include "Environment.h"

std::unique_ptr<WinImgReader> instance;

WinImgReader::WinImgReader()
{
    ocrInit();
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

void WinImgReader::readImg(Message* msg)
{
    JsonArray lineArr;
    JsonArray wordArr;
    auto arr = msg->result.GetNamedArray(L"$files");
    imgPath = arr.GetStringAt(0);
    
    msg->result.SetNamedValue(L"lines", lineArr);
    msg->result.SetNamedValue(L"words", wordArr);
    //co_await winrt::resume_foreground(Environment::get()->dq);
    msg->resolve();
}



