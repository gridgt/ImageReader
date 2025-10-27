#include <dwmapi.h>

#include "WindowBase.h"
#include "Message.h"
#include "Environment.h"
#include "WebView.h"

WindowBase::WindowBase()
{
}

WindowBase::~WindowBase()
{
}

void WindowBase::hittest(Message* msg)
{
    ReleaseCapture();
    int val = msg->param.GetNamedNumber(L"val");
    PostMessage(hwnd, WM_NCLBUTTONDOWN, val, 0);
    msg->resolve();
}

void WindowBase::minimize(Message* msg)
{
    webviewCtrl->NotifyParentWindowPositionChanged();
    HWND hwndWebView = FindWindowEx(hwnd, nullptr, L"Chrome_WidgetWin_0", nullptr);
    PostMessage(hwndWebView, WM_MOUSELEAVE, 0, 0);
    HWND hwndInner = FindWindowEx(hwndWebView, nullptr, NULL, nullptr);
    PostMessage(hwndInner, WM_MOUSELEAVE, 0, 0);
    ShowWindow(hwnd, SW_MINIMIZE);
    msg->resolve();
}

void WindowBase::maximize(Message* msg)
{
    ShowWindow(hwnd, SW_MAXIMIZE);
    msg->resolve();
}

void WindowBase::close(Message* msg)
{
    SendMessage(hwnd, WM_CLOSE, 0, 0);
    msg->resolve();
}

void WindowBase::restore(Message* msg)
{
    ShowWindow(hwnd, SW_RESTORE);
    msg->resolve();
}

void WindowBase::show(Message* msg)
{
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    msg->resolve();
}

void WindowBase::exec(Message* msg)
{
    auto methodName = msg->param.GetNamedString(L"_methodName");
    if (methodName == L"minimize") {
        minimize(msg);
    }
    else if (methodName == L"maximize")
    {
        maximize(msg);
    }
    else if (methodName == L"close")
    {
        close(msg);
    }
    else if (methodName == L"restore")
    {
        restore(msg);
    }
    else if (methodName == L"show")
    {
        show(msg);
    }
    else if (methodName == L"hittest")
    {
        hittest(msg);
    }
    else if (methodName == L"on")
    {
        on(msg);
    }
    else if (methodName == L"off")
    {
        off(msg);
    }
}

void WindowBase::on(Message* msg)
{
    auto eName = msg->param.GetNamedString(L"_eventName");
    eventTargets[eName].push_back(msg);
    msg->resolve();
}

void WindowBase::off(Message* msg)
{
    auto eName = msg->param.GetNamedString(L"_eventName");
    auto itMap = eventTargets.find(eName);
    if (itMap == eventTargets.end()) {
        msg->resolve();
        return;
    }
    auto& targets = itMap->second;
    auto it = std::remove_if(targets.begin(), targets.end(), [msg](Message* m) {
        if (m->sender == msg->sender) {
            delete m;
            return true;
        }
        return false;
        });
    targets.erase(it, targets.end());
    msg->resolve();
}

HRESULT WindowBase::ctrlReady(HRESULT result, ICoreWebView2Controller* ctrl)
{
    if (FAILED(result))
    {
        MessageBox(NULL, L"Failed to create webview2 controller", L"Error", MB_OK | MB_ICONERROR);
        ExitProcess(-1);
    }
    webviewCtrl = ctrl;
    RECT bounds;
    GetClientRect(hwnd, &bounds);
    webviewCtrl->put_Bounds(bounds);

    ComPtr<ICoreWebView2_22> webview22;
    {
        ComPtr<ICoreWebView2> webview;
        webviewCtrl->get_CoreWebView2(&webview);
        auto hr = webview.As(&webview22);
        if (FAILED(hr)) {
            auto result = MessageBox(NULL, L"WebView2系统组件版本过低，请安装新版本",
                L"系统提示", MB_OKCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON1);
            if (result == IDOK) {
                ShellExecute(0, 0, L"https://go.microsoft.com/fwlink/p/?LinkId=2124703", 0, 0, SW_SHOW);
            }
            ExitProcess(-1);
            return S_OK;
        }
    }
    this->webview = std::make_unique<WebView>(this, webview22.Get());
    return S_OK;
}

void WindowBase::createWindow()
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &WindowBase::winMsg;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_WINLOGO);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"ImageMarker";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_WINLOGO);
    RegisterClassExW(&wcex);
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, wcex.lpszClassName, wcex.lpszClassName, WS_OVERLAPPEDWINDOW,
        x, y, w, h, nullptr, nullptr, wcex.hInstance, nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    auto ctrlReadyInstance = Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(this, &WindowBase::ctrlReady);
    Environment::get()->env->CreateCoreWebView2Controller(hwnd, ctrlReadyInstance.Get());

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

void WindowBase::createShadow()
{
    MARGINS margins = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    int value = 2;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &value, sizeof(value));
    DwmSetWindowAttribute(hwnd, DWMWA_ALLOW_NCPAINT, &value, sizeof(value));
}

LRESULT WindowBase::winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto self = reinterpret_cast<WindowBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!self) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    if (msg == MSG_BACK_ID) {
        auto msg = reinterpret_cast<Message*>(lParam);
        msg->postMsgBack();
        return 0;
    }
    else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return self->procMsg(msg, wParam, lParam);
}
LRESULT WindowBase::procMsg(UINT msg, WPARAM wParam, LPARAM lParam) 
{
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
