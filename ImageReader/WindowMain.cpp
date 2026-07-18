#include "pch.h"
#include "D2D.h"
#include "Node.h"
#include "WindowMain.h"
#include "Util.h"
#include "Tip.h"
#include "TitleBar.h"
#include "Loader.h"
#include "WebSocket.h"

namespace {
    static std::unique_ptr<WindowMain> ins;
}
WindowMain::WindowMain() :WindowBase()
{
    setTitle(L"图像文字识别工具");
    setSize(800, 600);
    setPosScreenCenter();
}

WindowMain::~WindowMain()
{
}
void WindowMain::init()
{
    auto ptr = new WindowMain();
    ins.reset(ptr);
    ptr->createNativeWindow(0, WS_POPUP | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
    ptr->enableShadow();
    ptr->show();
}
WindowMain* WindowMain::get()
{
    return ins.get();
}
void WindowMain::onCreated()
{
    root->setBackgroundColor(0xFFFFFFFF);
    TitleBar::init(this);
    Loader::init(this);
}

void WindowMain::onMinMaxInfo(MINMAXINFO* mmi)
{
    RECT workAreaRect;
    BOOL getWorkAreaSuccess = SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
    mmi->ptMaxPosition.x = workAreaRect.left;
    mmi->ptMaxPosition.y = workAreaRect.top;
    mmi->ptMaxSize.x = workAreaRect.right - workAreaRect.left;
    mmi->ptMaxSize.y = workAreaRect.bottom - workAreaRect.top;
    mmi->ptMinTrackSize.x = 500; 
    mmi->ptMinTrackSize.y = 360;
}


LRESULT WindowMain::onHitTest(const float& x, const float& y)
{
    const float border = 4 * dpi;        // 拖动边框宽度（逻辑 4px）
    const float captionH = 30 * dpi;     // 标题栏高度（逻辑 30px）
    const float btnsW = 102 * dpi;       // 右上角按钮总宽
    bool left   = x < border;
    bool right  = x >= w - border;
    bool top    = y < border;
    bool bottom = y >= h - border;
    // 四个角优先
    if (top && left)     return HTTOPLEFT;
    if (top && right)    return HTTOPRIGHT;
    if (bottom && left)  return HTBOTTOMLEFT;
    if (bottom && right) return HTBOTTOMRIGHT;
    // 四条边
    if (left)   return HTLEFT;
    if (right)  return HTRIGHT;
    if (top)    return HTTOP;
    if (bottom) return HTBOTTOM;
    // 标题栏拖动区（避开右上角按钮）
    if (y < captionH && x < w - btnsW) {
        return HTCAPTION;
    }
    return HTCLIENT;
}
