#include <dwmapi.h>
#include "WinReader.h"
#include "Message.h"
#include "WinViewer.h"

std::unique_ptr<WinReader> winIns;

WinReader::WinReader()
{
    initPosSize();
    createWindow();
    transparentWindow();
    show();
}
WinReader::~WinReader() 
{
}

void WinReader::init()
{
	auto ins = new WinReader();
	winIns.reset(ins);
}

WinReader* WinReader::get()
{
	return winIns.get();
}

void WinReader::initPosSize()
{
    w = 500;
    h = 500;
    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMon, &mi);
    RECT work = mi.rcWork;
    int width = work.right - work.left;
    int height = work.bottom - work.top;
    x = work.left + (width - w) / 2;
    y = work.top + (height - h) / 2;
}

void WinReader::onViewReady()
{
    webview->Navigate(L"https://app.localhost/reader.html");
}

void WinReader::transparentWindow()
{
    HRGN region = CreateRectRgn(0, 0, -1, -1);
    DWM_BLURBEHIND bb = { 0 };
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = region;
    bb.fEnable = TRUE;
    DwmEnableBlurBehindWindow(hwnd, &bb);
    DeleteObject(region);
    DragAcceptFiles(hwnd, TRUE);
}

LRESULT WinReader::procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCHITTEST) {
        return HTCAPTION;
    }
    else if (msg == WM_DROPFILES) {
        onFileDrop((HDROP)wParam);
        return 0;
    }
    else if (msg == WM_GETMINMAXINFO) {
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

void WinReader::onFileDrop(HDROP hDrop)
{
    UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
    for (UINT i = 0; i < fileCount; ++i) {
        eventTargets[L"win_reading"]->resolve();
        //auto msg = eventTargets[L"win_end"];
        {
            TCHAR filePath[MAX_PATH];
            DragQueryFile(hDrop, i, filePath, MAX_PATH);
            WinViewer::init(filePath);
            //msg->result.SetNamedValue(L"imgPath", JsonValue::CreateStringValue(filePath));
        }
        //readImg(msg);
    }
    DragFinish(hDrop);
}

void WinReader::setMinMaxInfo(LPMINMAXINFO lpMMI)
{
    MONITORINFO mi = { sizeof(mi) };
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfo(hMonitor, &mi)) {
        lpMMI->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
        lpMMI->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
        lpMMI->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
        lpMMI->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
    }
    lpMMI->ptMinTrackSize.x = 600;
    lpMMI->ptMinTrackSize.y = 600;
}

void WinReader::onSize(UINT param)
{
    if (webviewCtrl)
    {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        webviewCtrl->put_Bounds(bounds);
    }
}

