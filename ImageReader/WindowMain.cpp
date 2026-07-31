#include "pch.h"
#include "WindowMain.h"
#include "TitleBar.h"
#include "WebSocket.h"
#include "ImgViewer.h"
#include "TextBox.h"
#include "StatusBar.h"

namespace {
    static std::unique_ptr<WindowMain> ins;
}
WindowMain::WindowMain() : Ling::WinBase()
{
    setTitle(L"图像控件演示");
    setSize(800, 600);
    setCenter();
    onDestroy.add([this] { Ling::App::get()->quit(); });
    createNativeWindow();
}

WindowMain::~WindowMain()
{
}

void WindowMain::init()
{
    Ling::init();
    Ling::D2D::get()->addFonts({ L"icon.ttf" });
    auto ptr = new WindowMain();
    ins.reset(ptr);
}

void WindowMain::onCreated()
{
    enableShadow();
    body->setBg(0xFFFFFFFF);
    body->setFlexDirection(Ling::FlexDirection::Column);
    titleBar = body->makeChild<TitleBar>();
    auto content = body->makeChild<Ling::Node>();
    content->setFlexGrow(1.f);
    content->setFlexDirection(Ling::FlexDirection::Row);
    imgViewer = content->makeChild<ImgViewer>();
    splitter = content->makeChild<Ling::Node>();
    splitter->setHeightPercent(100.f);
    splitter->setWidth(3.f);
    splitter->setBg(0xeeeeeeFF);
    textBox = content->makeChild<TextBox>();
    textBox->setWidth(260.f);
    statusBar = body->makeChild<StatusBar>();
    onCursor.add([this](bool* flag) {this->onSetCursor(flag);});
    onMouseDown.add([this](POINT pt, bool isRight) {this->onDown(pt,isRight);});
    onMouseMove.add([this](POINT pt) {this->onMove(pt);});
    onMouseUp.add([this](POINT pt,bool isRight) {this->onUp(pt);});
    show();
}

LRESULT WindowMain::onHitTest(const POINT pos)
{
    POINT pt = pos;
    ScreenToClient(hwnd, &pt);
    if (!isMaximized) {
        auto result = borderHitTest(pt);
        if (result != HTCLIENT) return result;
    }
    return titleBar->hitCaption(pt);
}

void WindowMain::onSetCursor(bool* flag)
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);
    if (!splitter->isPosIn(pt)) return;
    *flag = true;
    SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
}

void WindowMain::onDown(POINT pt, bool isRight)
{
    if (isRight) return;
    if (!splitter->isPosIn(pt)) return;
    // 落在 splitter 上：开始拖拽
    isDragging = true;
    SetCapture(hwnd);
}

void WindowMain::onMove(POINT pt)
{
    if (!isDragging) return;
    if (pt.x < 200) return;
    auto textBoxW{ (w - pt.x) / dpi };
    if (textBoxW < 120) return;

    textBox->setWidth((w-pt.x)/dpi);
    refresh();
}

void WindowMain::onUp(POINT pt)
{
    if (!isDragging) return;
    isDragging = false;
    ReleaseCapture();
}
