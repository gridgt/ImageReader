#include "pch.h"
#include "D2D.h"
#include "Node.h"
#include "WindowMain.h"
#include "Util.h"
#include "Tip.h"
#include "TitleBar.h"
#include "WebSocket.h"

namespace {
    static std::unique_ptr<WindowMain> win;
}
WindowMain::WindowMain() :WindowBase()
{
    setTitle(L"图像文字识别工具");
    setSize(800, 600);
    setPosScreenCenter();
    createNativeWindow(0, WS_POPUP | WS_MAXIMIZEBOX | WS_MINIMIZEBOX); //必须先创建窗口，再创建元素
    enableShadow();
    show();
}

WindowMain::~WindowMain()
{
}
void WindowMain::init()
{
    auto ptr = new WindowMain();
    win.reset(ptr);
}
WindowMain* WindowMain::get()
{
    return win.get();
}
void WindowMain::onCreated()
{
    root->setBackgroundColor(0xFF99FFFF);
    TitleBar::init(this);
}
void WindowMain::onMouseWheel(const int& x, const int& y, const short& delta)
{
    //for (auto& item : elements) item->onMouseWheel(x, y, delta);
}

void WindowMain::onTimer(const UINT& timerId)
{
    if (timerId == 66) {
    }
}


LRESULT WindowMain::onHitTest(const int& x, const int& y)
{
    if (x < w - 80 * dpi && y < 48 * dpi) {
        return HTCAPTION;
    }
    else {
        return HTCLIENT;
    }
}

BOOL WindowMain::setCursor()
{
    //for (auto& item : elements)
    //{
    //    if (item->setCursor()) {
    //        return TRUE;
    //    }
    //}
    //SetCursor(LoadCursor(NULL, IDC_ARROW));
    return TRUE;
}
