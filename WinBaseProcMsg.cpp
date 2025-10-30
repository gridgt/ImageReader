#include "WinBase.h"
#include "Message.h"

void WinBase::minimize(Message* msg)
{
    ShowWindow(hwnd, SW_MINIMIZE);
    msg->resolve();
}

void WinBase::maximize(Message* msg)
{
    ShowWindow(hwnd, SW_MAXIMIZE);
    msg->resolve();
}

void WinBase::close(Message* msg)
{
    SendMessage(hwnd, WM_CLOSE, 0, 0);
    msg->resolve();
}

void WinBase::restore(Message* msg)
{
    ShowWindow(hwnd, SW_RESTORE);
    msg->resolve();
}

void WinBase::show(Message* msg)
{
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    msg->resolve();
}

void WinBase::exec(Message* msg)
{
    auto methodName = msg->param.GetNamedString(L"$methodName");
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
    else if (methodName == L"on")
    {
        on(msg);
    }
    else if (methodName == L"off")
    {
        off(msg);
    }
}

void WinBase::on(Message* msg)
{
    auto eName = msg->param.GetNamedString(L"$eventName");
    if (eventTargets.contains(eName)) {
        msg->result.Remove(eName);
        msg->resolve();
        return;
    }
    eventTargets.insert({ eName,msg });
    msg->resolve();
}

void WinBase::off(Message* msg)
{
    auto eName = msg->param.GetNamedString(L"$eventName");
    if (eventTargets.contains(eName)) {
        eventTargets.erase(eName);
    }
    msg->result.Remove(eName);
    msg->resolve();
}