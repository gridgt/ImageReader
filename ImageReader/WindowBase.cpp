#include "pch.h"
#include "WindowBase.h"
#include "D2D.h"
#include "Node.h"
WindowBase::WindowBase() :Event(), compositor{ Composition::Compositor() }
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
    // 入参：逻辑像素；内部字段与 Win32 API：物理像素
    this->w = w * dpi;
    this->h = h * dpi;
    SetWindowPos(hwnd, nullptr, 0, 0,
        static_cast<int>(this->w), static_cast<int>(this->h),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
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
    // 入参：逻辑像素；字段：物理像素
    this->w = w * dpi;
    this->h = h * dpi;
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
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    int workWidth = workArea.right - workArea.left;
    int workHeight = workArea.bottom - workArea.top;

    // 工作区左上角偏移（处理多显示器时主屏可能不在 (0,0) 的情况）
    int workLeft = workArea.left;
    int workTop = workArea.top;

    // w/h 现在是物理像素，直接与屏幕坐标做算术
    x = workLeft + static_cast<int>((workWidth - w) / 2);
    y = workTop + static_cast<int>((workHeight - h) / 2);
}

void WindowBase::createNativeWindow(const DWORD& exStyle, const DWORD& style)
{
    // CreateWindowEx 用物理像素；字段 w/h 已经是物理
    hwnd = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP | exStyle, getWinClsName().data(), title.data(), style,
        x, y,
        static_cast<int>(w), static_cast<int>(h),
        NULL, NULL, GetModuleHandle(nullptr), NULL); //WS_POPUP
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    auto interop = compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();
    auto r = reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget**>(winrt::put_abi(winTarget));
    interop->CreateDesktopWindowTarget(hwnd, false, r);

    root = std::make_unique<Node>(this);
    winTarget.Root(root->visual);
    root->visual.Offset({ 0.f,0.f,0.f });
    root->visual.RelativeSizeAdjustment({ 1.f,1.f });   // root 跟随窗口客户区（物理）
    onCreated();
    root->sizeChange();
}

BOOL WindowBase::setCursor()
{
    for (auto* n = nodeHover; n; n = n->parent) {
        if (n->cursor) {
            ::SetCursor(n->cursor);
            return TRUE;
        }
    }
    ::SetCursor(LoadCursor(nullptr, IDC_ARROW));
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
        // WM_NCHITTEST：屏幕坐标（物理）减去窗口位置，得到客户区物理坐标
        float lx = static_cast<float>(GET_X_LPARAM(lParam) - self->x);
        float ly = static_cast<float>(GET_Y_LPARAM(lParam) - self->y);
        return self->onHitTest(lx, ly);
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
        self->mouseDown(static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam)), true);
    }
    else if (msg == WM_RBUTTONUP) {
        self->mouseUp(static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam)), true);
    }
    else if (msg == WM_LBUTTONDOWN) {
        self->mouseDown(static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam)), false);
    }
    else if (msg == WM_LBUTTONUP) {
        self->mouseUp(static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam)), false);
    }
    else if (msg == WM_MOUSEMOVE) {
        self->mouseMove(static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam)));
    }
    else if (msg == WM_MOUSELEAVE) {
        self->mouseLeave();
        return 0;
    }
    else if (msg == WM_MOUSEWHEEL) {
        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &pt);
        self->onMouseWheel(static_cast<float>(pt.x), static_cast<float>(pt.y), (short)HIWORD(wParam));
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
    else if (msg == WM_SYSCOMMAND) {

    }
    else if (msg == WM_SIZE) { 
        self->sizeChange(wParam, lParam);
        return 0;
    }
    else if (msg == WM_MOVE) {
        self->positionChange(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    else if (msg == WM_GETMINMAXINFO) {
        self->onMinMaxInfo((PMINMAXINFO)lParam);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WindowBase::mouseMove(const float& x, const float& y)
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
        for (auto* node : leavePath) {
            node->stopEventPopup = false;
            node->emit("mouseLeave", node);
            if (node->stopEventPopup) {
                node->stopEventPopup = false;
                break;
            }
        }
    }
    nodeHover = hit;
    if (nodeHover) {
        auto enterPath = nodeHover->pathUpTo(lca); 
        for (auto node : enterPath) {
            node->stopEventPopup = false;
            auto arg = std::make_tuple(x, y, node);
            node->emit("mouseEnter", &arg);
            if (node->stopEventPopup) {
                node->stopEventPopup = false;
                break;
            }
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
        auto path = nodeHover->pathUpTo(nullptr); // 冒泡到根
        for (auto* node : path) {
            node->stopEventPopup = false;
            node->emit("mouseLeave", node);
            if (node->stopEventPopup) {
                node->stopEventPopup = false;
                break;
            }
        }
    }
    nodeHover = nullptr;
}

void WindowBase::mouseDown(const float& x, const float& y, bool isRight)
{
    if (!nodeHover) return;
    auto path = nodeHover->pathUpTo(nullptr);
    for (auto* node : path) {
        node->stopEventPopup = false;
        auto arg = std::make_tuple(x, y, isRight, node);
        node->emit("mouseDown", &arg);
        if (node->stopEventPopup) {
            node->stopEventPopup = false;
            break;
        }
    }
}

void WindowBase::mouseUp(const float& x, const float& y, bool isRight)
{
    if (!nodeHover) return;
    auto path = nodeHover->pathUpTo(nullptr);
    for (auto* node : path) {
        node->stopEventPopup = false;
        auto arg = std::make_tuple(x, y, isRight, node);
        node->emit("mouseUp", &arg);
        if (node->stopEventPopup) {
            node->stopEventPopup = false;
            break;
        }
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

void WindowBase::sizeChange(WPARAM wParam, LPARAM lParam)
{
    if (wParam == SIZE_MINIMIZED) {
        mouseLeave();
        emit("minimize", nullptr);
        return;
    }
    if (wParam == SIZE_MAXIMIZED) {
        wasMaximized = true;
        emit("maximize", nullptr);
    }
    else if (wParam == SIZE_RESTORED) {
        if (wasMaximized) {
            wasMaximized = false;
            emit("restore", nullptr);
        }
    }
    w = static_cast<float>(GET_X_LPARAM(lParam));
    h = static_cast<float>(GET_Y_LPARAM(lParam));
    if (w <= 0 || h <= 0) return;
    root->sizeChange();
}
void WindowBase::positionChange(const int& x, const int& y)
{
    this->x = x;
    this->y = y;
    onPositionChange();
}
