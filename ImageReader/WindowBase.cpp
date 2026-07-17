#include "pch.h"
#include "WindowBase.h"
#include "D2D.h"
#include "Node.h"
#include "EventArg.h"
#include "MouseEventArg.h"
WindowBase::WindowBase() :compositor{ Composition::Compositor() }
{
    dpi = static_cast<float>(GetDpiForSystem()) / 96.f;
}
WindowBase::~WindowBase()
{
}
void WindowBase::show()
{
    ShowWindow(hwnd, SW_SHOW);
}
void WindowBase::hide()
{
    ShowWindow(hwnd, SW_HIDE);
}
void WindowBase::onHidden()
{
    isMouseIn = false;
    mouseLeave();
}
void WindowBase::refresh()
{
    InvalidateRect(hwnd, nullptr, false);
}
void WindowBase::close()
{
    SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
    DestroyWindow(hwnd);
    onDestroy();
}
void WindowBase::minimize()
{
    ShowWindow(hwnd, SW_MINIMIZE);
}
void WindowBase::move(const int& x, const int& y)
{
    this->x = x;
    this->y = y;
    SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
}

void WindowBase::resize(const int& w, const int& h)
{
    this->w = w*dpi;
    this->h = h*dpi;
    SetWindowPos(hwnd, nullptr, 0, 0, this->w, this->h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
}



void WindowBase::enableShadow()
{
    MARGINS margins = { 1,1,1,1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    int value = 2;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &value, sizeof(value));
    DwmSetWindowAttribute(hwnd, DWMWA_ALLOW_NCPAINT, &value, sizeof(value));
    //DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
    //DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
}
void WindowBase::setTimer(const UINT& elapse, const UINT& id)
{
    SetTimer(hwnd, WM_APP + id, elapse, nullptr);
}

void WindowBase::killTimer(const UINT& id)
{
    KillTimer(hwnd, WM_APP + id);
}


void WindowBase::setTitle(const std::wstring& title)
{
    this->title = title;
    if (hwnd) {
        SetWindowText(hwnd, title.data());
    }
}

std::wstring WindowBase::getTitle()
{
    return title;
}
std::tuple<int, int> WindowBase::getPosition()
{
    return std::make_tuple(x, y);
}

std::tuple<float, float> WindowBase::getSize()
{
    return std::make_tuple(w, h);
}
void WindowBase::setSize(const float& w, const float& h)
{
    this->w = w*dpi;
    this->h = h*dpi;
}

void WindowBase::setPosition(const int& x, const int& y)
{
    this->x = x;
    this->y = y;
}
HWND WindowBase::getHandle()
{
    return hwnd;
}
float WindowBase::getScaleFactor()
{
    return dpi;
}
void WindowBase::setPosScreenCenter()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    x = (screenWidth - w) / 2;
    y = (screenHeight - h) / 2;
}

void WindowBase::createNativeWindow(const DWORD& exStyle, const DWORD& style)
{
    hwnd = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP | exStyle, getWinClsName().data(), title.data(), style, x, y, w, h, NULL, NULL, GetModuleHandle(nullptr), NULL); //WS_POPUP
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    auto interop = compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();
    auto r = reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget**>(winrt::put_abi(winTarget));
    interop->CreateDesktopWindowTarget(hwnd, false, r);

    root = std::make_unique<Node>(this);
    winTarget.Root(root->visual);
    root->visual.Offset({ 0.f,0.f,0.f });
    root->visual.RelativeSizeAdjustment({ 1.f,1.f });   
    onCreated();
    root->sizeChange();
}

BOOL WindowBase::setCursor()
{
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return TRUE;
}

std::wstring& WindowBase::getWinClsName()
{
    static std::wstring clsName = [] {
        WNDCLASSEXW wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = &WindowBase::winProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = GetModuleHandle(nullptr);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"ImageReader";
        wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(100));  // 任务栏大图标
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(100));  // 标题栏小图标
        auto r = RegisterClassEx(&wcex);
        if (r == 0) {
            log(L"err:: reg window class error");
        }
        return wcex.lpszClassName;
        }();
    return clsName;
}

LRESULT WindowBase::winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto self = reinterpret_cast<WindowBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!self) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    else if (msg == WM_NCHITTEST) {
        return self->onHitTest(GET_X_LPARAM(lParam) - self->x, GET_Y_LPARAM(lParam) - self->y);
    }
    else if (msg == WM_ERASEBKGND) {
        return 1;
    }
    else if (msg == WM_SHOWWINDOW) {
        if (wParam) {
            self->onShown();
        }
        else {
            self->onHidden();
        }
    }
    else if (msg == WM_SETCURSOR) {
        if (LOWORD(lParam) == HTCLIENT) return self->setCursor();
    }
    else if (msg == WM_RBUTTONDOWN) {
        self->mouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), true);
    }
    else if (msg == WM_RBUTTONUP) {
        self->mouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), true);
    }
    else if (msg == WM_LBUTTONDOWN) {
        self->mouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), false);
    }
    else if (msg == WM_LBUTTONUP) {
        self->mouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),false);
    }
    else if (msg == WM_MOUSEMOVE) {
        self->mouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    else if (msg == WM_MOUSELEAVE) {
        self->mouseLeave();
        return 0;
    }
    else if (msg == WM_MOUSEWHEEL) {
        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &pt);
        self->onMouseWheel(pt.x, pt.y, (short)HIWORD(wParam));
        return 0;
    }
    else if (msg == WM_KEYDOWN) {
        self->onKeyDown(wParam);
    }
    else if (msg == WM_KEYUP) {
        self->onKeyUp();
    }
    else if (msg == WM_CHAR) {
        self->onChar(wParam);
    }
    else if (msg == WM_IME_STARTCOMPOSITION) {
        self->onIme();
    }
    else if (msg == WM_TIMER) {
        self->onTimer(wParam - WM_APP);
    }
    else if (msg == WM_KILLFOCUS) {
        self->onBlur();
    }
    else if (msg == WM_DPICHANGED) {
        self->dpiChange(wParam, lParam);
    }
    else if (msg == WM_SIZE) {
        if (wParam == SIZE_MINIMIZED) {
            self->mouseLeave();
        }
        self->sizeChange(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    }
    else if (msg == WM_MOVE) {
        self->positionChange(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WindowBase::mouseMove(const int& x, const int& y)
{
    if (!isMouseIn) {
        isMouseIn = true;
        TRACKMOUSEEVENT tme{ sizeof(TRACKMOUSEEVENT) };
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        TrackMouseEvent(&tme);
    }
    auto hit = root->findLeafByPos(x, y);
    if (hit == nodeHover) {        
        return;// 没有变化，什么都不做
    }
    auto lca = hit->findLCA(nodeHover); //找最近公共祖先
    if (nodeHover) {
        auto leavePath = nodeHover->pathUpTo(lca);
        EventArg arg;
        for (auto* node : leavePath) {
            node->mouseLeave(arg);
            if (arg.stopPopup) break;
        }
    }
    nodeHover = hit;
    if (nodeHover) {
        auto enterPath = nodeHover->pathUpTo(lca);
        std::reverse(enterPath.begin(), enterPath.end());
        MouseEventArg arg(x, y, false);
        for (auto* node : enterPath) {
            node->mouseEnter(arg);
            if (arg.stopPopup) break;
        }
    }
}

void WindowBase::mouseLeave()
{
    isMouseIn = false;
    TRACKMOUSEEVENT tme{ sizeof(TRACKMOUSEEVENT) };
    tme.dwFlags = TME_CANCEL | TME_LEAVE;
    tme.hwndTrack = hwnd;
    TrackMouseEvent(&tme);
    if (nodeHover) {
        EventArg arg;
        auto path = nodeHover->pathUpTo(nullptr); // 冒泡到根
        for (auto* node : path) {
            node->mouseLeave(arg);
            if (arg.stopPopup) break;
        }
    }
    nodeHover = nullptr;
}

void WindowBase::mouseDown(const int& x, const int& y, bool isRight)
{
    if (!nodeHover) return;
    auto path = nodeHover->pathUpTo(nullptr);
    MouseEventArg arg(x, y, isRight);
    for (auto* node : path) {
        node->mouseDown(arg);
        if (arg.stopPopup) break;
    }
}

void WindowBase::mouseUp(const int& x, const int& y, bool isRight)
{
    if (!nodeHover) return;
    auto path = nodeHover->pathUpTo(nullptr);
    MouseEventArg arg(x, y, isRight);
    for (auto* node : path) {
        node->mouseUp(arg);
        if (arg.stopPopup) break;
    }
}

void WindowBase::dpiChange(WPARAM wParam, LPARAM lParam)
{
    const UINT newDPI = HIWORD(wParam);
    dpi = newDPI / 96.f;
    RECT* prcNewWindow = reinterpret_cast<RECT*>(lParam);
    auto w{ prcNewWindow->right - prcNewWindow->left };
    auto h{ prcNewWindow->bottom - prcNewWindow->top };
    SetWindowPos(hwnd, nullptr, prcNewWindow->left, prcNewWindow->top, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
    positionChange(prcNewWindow->left, prcNewWindow->top);
    sizeChange(w, h);
    onDpiChanged();
}

void WindowBase::sizeChange(const int& w, const int& h)
{
    this->w = w;
    this->h = h;
    root->sizeChange();
}
void WindowBase::positionChange(const int& x, const int& y)
{
    this->x = x;
    this->y = y;
    onPositionChange();
}
