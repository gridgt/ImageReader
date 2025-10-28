#include "WindowMain.h"
#include "Message.h"

void WindowMain::hittest(Message* msg)
{
    ReleaseCapture();
    int val = msg->param.GetNamedNumber(L"val");
    PostMessage(hwnd, WM_NCLBUTTONDOWN, val, 0);
    msg->resolve();
}

void WindowMain::minimize(Message* msg)
{
    webviewCtrl->NotifyParentWindowPositionChanged();
    HWND hwndWebView = FindWindowEx(hwnd, nullptr, L"Chrome_WidgetWin_0", nullptr);
    PostMessage(hwndWebView, WM_MOUSELEAVE, 0, 0);
    HWND hwndInner = FindWindowEx(hwndWebView, nullptr, NULL, nullptr);
    PostMessage(hwndInner, WM_MOUSELEAVE, 0, 0);
    ShowWindow(hwnd, SW_MINIMIZE);
    msg->resolve();
}

void WindowMain::maximize(Message* msg)
{
    ShowWindow(hwnd, SW_MAXIMIZE);
    msg->resolve();
}

void WindowMain::close(Message* msg)
{
    SendMessage(hwnd, WM_CLOSE, 0, 0);
    msg->resolve();
}

void WindowMain::restore(Message* msg)
{
    ShowWindow(hwnd, SW_RESTORE);
    msg->resolve();
}

void WindowMain::show(Message* msg)
{
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    msg->resolve();
}

void WindowMain::exec(Message* msg)
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

void WindowMain::on(Message* msg)
{
    auto eName = msg->param.GetNamedString(L"_eventName");
    eventTargets[eName].push_back(msg);
    msg->resolve();
}

void WindowMain::off(Message* msg)
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