#include "pch.h"
#include "Env.h"
#include "Window.h"
#include "Page.h"

std::unordered_map<HWND, std::unique_ptr<Window>> windows;
Window::Window(const std::wstring& url) :url{ url }
{
}

Window::~Window()
{
}
Window* Window::create(const std::wstring& url)
{
    auto win = std::make_unique<Window>(url);
    win->createWin();
    auto result = win.get();
    windows.insert({ win->hwnd ,std::move(win) });
    return result;
}
LRESULT Window::winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto self = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!self) return DefWindowProc(hwnd, msg, wParam, lParam);
    if (msg == WM_SIZE) {
        self->onSize(LOWORD(lParam), HIWORD(lParam));
    }
    else if (msg == WM_DESTROY) {
        self->onDestroy();
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Window::createWin()
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &Window::winMsg;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_WINLOGO);
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_WINLOGO);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"Sample";
    RegisterClassEx(&wcex);
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, wcex.lpszClassName, wcex.lpszClassName, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, nullptr, nullptr, wcex.hInstance, nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    auto wvEnv = Env::getWebViewEnv();
    auto ctrlReadyCB = Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(this, &Window::onCtrlReady);
    wvEnv->CreateCoreWebView2Controller(hwnd, ctrlReadyCB.Get());
    ShowWindow(hwnd, SW_SHOW);
}

void Window::show()
{
    ShowWindow(hwnd, SW_SHOW);
    ctrl->put_IsVisible(TRUE);
}

bool Window::hasWindow(const HWND hwnd)
{
    return windows.contains(hwnd);
}

HRESULT Window::onCtrlReady(HRESULT result, ICoreWebView2Controller* ctrl)
{
    this->ctrl = ctrl;
    ICoreWebView2* webview;
    ctrl->get_CoreWebView2(&webview);
    RECT bounds;
    GetClientRect(hwnd, &bounds);
    ctrl->put_Bounds(bounds);
    page = std::make_unique<Page>(this, webview);
    page->navigate(url);
    return S_OK;
}
void Window::onSize(int w, int h)
{
    if (!ctrl.Get()) return;
    RECT bounds = { 0, 0, w, h };
    ctrl->put_Bounds(bounds);
}
void Window::onDestroy()
{
    windows.erase(hwnd);
    if (windows.empty()) {
        PostQuitMessage(0);
    }
}

