#include "WindowMain.h"
#include "WebView.h"
#include "Message.h"

std::unique_ptr<WindowMain> winIns;

WindowMain::WindowMain() :WindowBase()
{
    x = 100;
    y = 100;
    h = 1200;
    w = 1000;
    createWindow();
}

WindowMain::~WindowMain()
{
}

void WindowMain::init()
{
    auto ins = new WindowMain();
    winIns.reset(ins);
}

LRESULT WindowMain::procMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_GETMINMAXINFO:
        {
            setMinMaxInfo((LPMINMAXINFO)lParam);
            return 0;
        }
        case WM_SIZE:
        {
            onSize(wParam);
            return 0;
        }
    }
    return WindowBase::procMsg(msg, wParam, lParam);
}

void WindowMain::setMinMaxInfo(LPMINMAXINFO lpMMI)
{
    MONITORINFO mi = { sizeof(mi) };
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfo(hMonitor, &mi)) {
        lpMMI->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
        lpMMI->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
        lpMMI->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
        lpMMI->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
    }
    lpMMI->ptMinTrackSize.x = 1000;
    lpMMI->ptMinTrackSize.y = 800;
}

void WindowMain::onSize(UINT param)
{
    if (param == SIZE_MAXIMIZED) {
        auto& targets = eventTargets[L"win_maximize"];
        for (auto& msg : targets)
        {
            msg->resolve();
        }
    }
    else if (param == SIZE_RESTORED) {
        auto& targets = eventTargets[L"win_restore"];
        for (auto& msg : targets)
        {
            msg->resolve();
        }
    }
    if (webviewCtrl)
    {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        webviewCtrl->put_Bounds(bounds);
    }
}




