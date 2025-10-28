#include <dwmapi.h>

#include "WindowMain.h"
#include "Message.h"
#include "Environment.h"

std::unique_ptr<WindowMain> winIns;

WindowMain::WindowMain()
{
    createCompCtrl();
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

WindowMain* WindowMain::get()
{
    return winIns.get();
}


void WindowMain::createWindow()
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &WindowMain::winMsg;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_WINLOGO);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"ImageReader";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_WINLOGO);
    RegisterClassExW(&wcex);


    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    int x, y;
    GetMonitorInfo(hMon, &mi);
    RECT work = mi.rcWork;
    int width = work.right - work.left;
    int height = work.bottom - work.top;
    x = work.left + (width - w) / 2;
    y = work.top + (height - h) / 2;
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, wcex.lpszClassName, wcex.lpszClassName, WS_POPUP,
        x, y, w, h, nullptr, nullptr, wcex.hInstance, nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    //MARGINS margins = { 1, 1, 1, 1 };
    //DwmExtendFrameIntoClientArea(hwnd, &margins);
    //int value = 2;
    //DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &value, sizeof(value));
    //DwmSetWindowAttribute(hwnd, DWMWA_ALLOW_NCPAINT, &value, sizeof(value));


    HRGN region = CreateRectRgn(0, 0, -1, -1);
    DWM_BLURBEHIND bb = { 0 };
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = region;
    bb.fEnable = TRUE;
    DwmEnableBlurBehindWindow(hwnd, &bb);
    DeleteObject(region);

    DragAcceptFiles(hwnd, TRUE);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

LRESULT WindowMain::winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto self = reinterpret_cast<WindowMain*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!self) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    if (msg == MSG_BACK_ID) {
        auto msg = reinterpret_cast<Message*>(lParam);
        msg->postMsgBack();
        return 0;
    }
    else if (msg == WM_ERASEBKGND) {
        return 0;
    }
    else if (msg == WM_NCHITTEST) {
        return HTCAPTION;
    }
    else if (msg == WM_DROPFILES) {
        HDROP hDrop = (HDROP)wParam;
        UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
        for (UINT i = 0; i < fileCount; ++i) {
            TCHAR filePath[MAX_PATH];
            DragQueryFile(hDrop, i, filePath, MAX_PATH);
            // 处理文件路径，例如显示 MessageBox
            //MessageBox(hwnd, filePath, TEXT("Dropped File"), MB_OK);
            auto& targets = self->eventTargets[L"win_reading"];
            for (auto& msg : targets)
            {
                msg->resolve();
            }
        }
        DragFinish(hDrop);  // 释放资源
    }
    else if (msg == WM_GETMINMAXINFO) {
        self->setMinMaxInfo((LPMINMAXINFO)lParam);
        return 0;
    }
    else if (msg == WM_SIZE) {
        self->onSize(wParam);
        return 0;
    }
    else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
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
    lpMMI->ptMinTrackSize.x = 600;
    lpMMI->ptMinTrackSize.y = 600;
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
