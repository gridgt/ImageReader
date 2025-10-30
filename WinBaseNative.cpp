#include <dwmapi.h>
#include "WinBase.h"
#include "Message.h"
#include "Environment.h"

WinBase::WinBase()
{
    createCompCtrl();
}

WinBase::~WinBase()
{

}

void WinBase::createWindow()
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &WinBase::winMsg;
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
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, wcex.lpszClassName, wcex.lpszClassName, WS_POPUP,
        x, y, w, h, nullptr, nullptr, wcex.hInstance, nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));   
    dpi = GetDpiForWindow(hwnd) / 96.0f;
}

void WinBase::show()
{
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

LRESULT WinBase::winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto self = reinterpret_cast<WinBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!self) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    if (msg == WM_ERASEBKGND) {
        return 0;
    }
    else if (msg == WM_DPICHANGED) {
        self->dpi = LOWORD(wParam) / 96.0f;
        return 0;
    }
    else if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
    {
        self->routeMsgToPage(msg, wParam, lParam);
        return 1;
    }
    else if (msg == WM_MOUSELEAVE)
    {
        self->mouseLeave();
        return 1;
    }
    return self->procNativeMsg(msg, wParam, lParam);
}

LRESULT WinBase::procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WinBase::mouseLeave()
{
    isMouseTracking = false;
    if (!ctrlComp) return;
    ctrlComp->SendMouseInput(COREWEBVIEW2_MOUSE_EVENT_KIND_LEAVE,
        COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE, 0, POINT{});
}

void WinBase::routeMsgToPage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!ctrlComp) return;
    DWORD mouseData = 0;
    POINT point{ .x{GET_X_LPARAM(lParam)},.y{GET_Y_LPARAM(lParam)} };
    if (msg == WM_MOUSEMOVE)
    {
        if (!isMouseTracking) {
            TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            isMouseTracking = true;
        }
    }
    else if (msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL)
    {
        mouseData = GET_WHEEL_DELTA_WPARAM(wParam);
        ScreenToClient(hwnd, &point);
    }
    else if (msg == WM_XBUTTONDBLCLK ||
        msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP) {
        mouseData = GET_XBUTTON_WPARAM(wParam);
    }
    auto eventKind = static_cast<COREWEBVIEW2_MOUSE_EVENT_KIND>(msg);
    auto eventKey = static_cast<COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS>(GET_KEYSTATE_WPARAM(wParam));
    ctrlComp->SendMouseInput(eventKind, eventKey, mouseData, point);
}
