#include "pch.h"
#include <shellscalingapi.h>
#include "D2D.h"
#include "Skin.h"
#include "ElementBase.h"
#include "WindowCalendar.h"
#include "TitleBtn.h"
#include "YearBtn.h"
#include "YearText.h"
#include "WeekHeader.h"
#include "DayBtn.h"
#include "Util.h"
#include "ListBar.h"
#include "ListContent.h"
#include "SwitchBtn.h"
#include "TodayBtn.h"
#include "Menu.h"
#include "Tip.h"
#include "WebSocket.h"

namespace {
    static std::unique_ptr<WindowCalendar> win;
    WNDPROC oldProc;
}
WindowCalendar::WindowCalendar() :WindowBase()
{
    setTitle(L"日历");
    setSize(logicW, logicH);
    setPosScreenCenter();
    createNativeWindow(0, WS_POPUP | WS_MAXIMIZEBOX | WS_MINIMIZEBOX); //必须先创建窗口，再创建元素
    show();
}

WindowCalendar::~WindowCalendar()
{
}
void WindowCalendar::init()
{
    auto ptr = new WindowCalendar();
    win.reset(ptr);
}
WindowCalendar* WindowCalendar::get()
{
    return win.get();
}
void WindowCalendar::switchEmbed()
{
    if (isEmbeded) {
        unembed();
    }
    else {
        embed();
    }
    onMouseMove(-999999, -999999);
    isEmbeded = !isEmbeded;
}
void WindowCalendar::hideSchedule()
{
    auto listBar = getChildById(L"listBar");
    auto listContent = getChildById(L"listContent");
    listBar->hide();
    listContent->hide();
    logicH = 480;
    resize(logicW, logicH);
}
void WindowCalendar::showSchedule()
{
    auto listBar = getChildById(L"listBar");
    auto listContent = getChildById(L"listContent");
    listBar->show();
    listContent->show();
    logicH = 680;
    resize(logicW, logicH);
}



void WindowCalendar::onCreated()
{
    elements.push_back(std::make_unique<TitleBtn>(this,L"titleMenuBtn", 45.f,L"\ue6e8"));
    elements.push_back(std::make_unique<TitleBtn>(this,L"titlePinBtn",80.f, L"\ue74c"));
    elements.push_back(std::make_unique<YearBtn>(this,L"yearLeftBtn",80.f, L"\ue709"));
    elements.push_back(std::make_unique<YearBtn>(this,L"yearRightBtn",240.f, L"\ue70e"));
    elements.push_back(std::make_unique<YearText>(this,L"yearText"));
    elements.push_back(std::make_unique<TodayBtn>(this, L"todayBtn"));
    for (size_t i = 0; i < 7; i++)
    {
        auto id = std::format(L"weekHeader{}", i);
        elements.push_back(std::make_unique<WeekHeader>(this, id, i));
    }
    for (size_t i = 0; i < 42; i++)
    {
        auto id = std::format(L"dayBtn{}", i);
        elements.push_back(std::make_unique<DayBtn>(this, id, i));
    }
    elements.push_back(std::make_unique<ListBar>(this, L"listBar"));
    elements.push_back(std::make_unique<ListContent>(this, L"listContent"));
    elements.push_back(std::make_unique<SwitchBtn>(this, L"switchBtn"));
    elements.push_back(std::make_unique<Menu>(this, L"menu"));
    elements.push_back(std::make_unique<Tip>(this, L"tip"));
}
void WindowCalendar::onSizeChange()
{
    if (w == 0 || h == 0 || !hasData) return;
    for (auto& item: elements)
    {
        item->onSizeChange();
    }
}

void WindowCalendar::onMouseDown(const int& x, const int& y, bool isRight)
{
    //if (y<48) {
    //    pressPos.x = x;
    //    pressPos.y = y;
    //}
    for (auto& item : elements)
    {
        item->onMouseDown(x, y,isRight);
    }
}

void WindowCalendar::onMouseMove(const int& x, const int& y)
{
    for (auto& item : elements)
    {
        item->onMouseMove(x,y);
    }
}

void WindowCalendar::onMouseUp(const int& x, const int& y)
{
    for (auto& item : elements) item->onMouseUp(x, y);
}
void WindowCalendar::onMouseDrag(const int& x, const int& y, const UINT_PTR& modifiers)
{
    for (auto& item : elements) item->onMouseDrag(x, y, modifiers);
}
void WindowCalendar::onMouseWheel(const int& x, const int& y, const short& delta)
{
    for (auto& item : elements) item->onMouseWheel(x, y, delta);
}

void WindowCalendar::onMouseLeave()
{
    for (auto& item : elements)
    {
        item->onMouseMove(-999999, -999999);
    }
}

void WindowCalendar::onTimer(const UINT& timerId)
{
    if (timerId == 66) {
        auto titleMenuBtn = dynamic_cast<TitleBtn*>(getChildById(L"titleMenuBtn"));
        auto menu = dynamic_cast<Menu*>(getChildById(L"menu"));
        if (menu->hoverIndex < 0 && !titleMenuBtn->isHover) {
            menu->hide();
            killTimer(66);
        }
    }
}


LRESULT WindowCalendar::onHitTest(const int& x, const int& y)
{
    if (x < win->w - 80 * win->dpi && y < 48 * win->dpi) {
        return HTCAPTION;
    }
    else {
        return HTCLIENT;
    }
}

void WindowCalendar::onData(const JsonObject& json)
{
    if (json.GetNamedString(L"msgName") == L"showToast") {
        return;
    }
    auto jsonObj = json.GetNamedObject(L"data");
    if (jsonObj.HasKey(L"displayScheduleList")) {
        auto val = jsonObj.GetNamedValue(L"displayScheduleList");
        bool flag{ false };
        if (val.ValueType() == JsonValueType::String) {
            flag = val.GetString() == L"true";
        }
        else {
            flag = val.GetBoolean();
        }
        if (!flag && win->h > 480 * win->dpi) {
            hideSchedule();
        }
        if (flag && win->h < 680 * win->dpi) {
            showSchedule();
        }
    }
    auto flag = hasData;
    hasData = true;
    if (jsonObj.HasKey(L"embedPosition")) {
        auto posData = jsonObj.GetNamedObject(L"embedPosition");
        embedPosition.x = (int)posData.GetNamedNumber(L"x");
        embedPosition.y = (int)posData.GetNamedNumber(L"y");
        move(embedPosition.x, embedPosition.y);
    }
    auto lang = jsonObj.GetNamedObject(L"lang");
    clickToCompleteTodo = lang.GetNamedString(L"clickToCompleteTodo");
    clickToRestartTodo = lang.GetNamedString(L"clickToRestartTodo");

    auto skin = Skin::get();
    skin->onData(jsonObj);
    rootVisual.Brush(compositor.CreateColorBrush(skin->bg.getUIColor()));
    for (auto& item : elements)
    {
        item->onData(jsonObj);
    }
    onSizeChange();
    if (!flag) {
        switchEmbed();
    }
}

ElementBase* WindowCalendar::getChildById(const std::wstring& id)
{
    for (auto& ele:elements)
    {
        if (ele->id == id) {
            return ele.get();
        }
    }
    return nullptr;
}

BOOL WindowCalendar::setCursor()
{
    for (auto& item : elements)
    {
        if (item->setCursor()) {
            return TRUE;
        }
    }
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return TRUE;
}
void WindowCalendar::embed()
{
    HWND workerW = Util::getWorkerW();
    RECT before;
    GetWindowRect(win->hwnd, &before);
    embedPosition.x = before.left;
    embedPosition.y = before.top;
    SetParent(win->hwnd, workerW);
    // WorkerW 覆盖整个虚拟桌面，其 (0,0) 位于虚拟桌面左上角，
    // 若存在位于主屏上方的副屏，该原点会在主屏之上（y 为负），
    // 因此需要把目标屏幕坐标转换为 WorkerW 客户区坐标，再重新定位。
    POINT pt{ before.left, before.top };
    ScreenToClient(workerW, &pt);
    SetWindowPos(win->hwnd, nullptr, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
    Util::regInputDevice(win->hwnd);
    oldProc = (WNDPROC)SetWindowLongPtr(win->hwnd, GWLP_WNDPROC, (LONG_PTR)WindowCalendar::processMsg);

//    auto str = std::format(LR"({{
//"msgType":"EmbedCalendar","msgName":"embedWin","data":{{"x":{},"y":{}}}
//}})", embedPosition.x, embedPosition.y);
//    WebSocket::get()->sendMsg(str);
}

void WindowCalendar::unembed()
{
    Util::regInputDevice(nullptr);
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldProc);
    SetParent(hwnd, nullptr);

    // 位于两屏幕接缝处时，MonitorFromPoint 只按左上角一点探测显示器，
    // 而 Windows 自身按主要面积决定窗口归属的显示器；两者若不一致，
    // 直接按探测到的 DPI 计算尺寸会在 SetWindowPos 期间触发 WM_DPICHANGED，
    // 该消息又会用 Windows 建议的矩形覆盖尺寸，最终导致大小错乱。
    // 所以先只移动到目标位置，让系统敲定窗口归属，再取实际 DPI 计算尺寸。
    SetWindowPos(hwnd, nullptr, embedPosition.x, embedPosition.y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
    x = embedPosition.x;
    y = embedPosition.y;

    UINT actualDpi = GetDpiForWindow(hwnd);
    if (actualDpi == 0) actualDpi = 96;
    dpi = actualDpi / 96.f;
    int newW = static_cast<int>(logicW * dpi);
    int newH = static_cast<int>(logicH * dpi);
    SetWindowPos(hwnd, HWND_TOP, embedPosition.x, embedPosition.y, newW, newH,
        SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    // 应用新尺寸时若主要面积滑到另一块屏幕仍会再触发 WM_DPICHANGED，
    // WindowBase 的默认处理器会按建议矩形改写窗口大小。这里再复核一次
    // 当前真实 DPI，把最终尺寸强制回到 logic * 实际 DPI，保证内外一致。
    actualDpi = GetDpiForWindow(hwnd);
    if (actualDpi == 0) actualDpi = 96;
    dpi = actualDpi / 96.f;
    newW = static_cast<int>(logicW * dpi);
    newH = static_cast<int>(logicH * dpi);
    SetWindowPos(hwnd, nullptr, 0, 0, newW, newH,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    sizeChange(newW, newH);
    ReleaseCapture();
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    isMouseIn = false;
    isMouseDown = false;
}

LRESULT CALLBACK WindowCalendar::processMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg != WM_INPUT) {
        return CallWindowProc(oldProc, hWnd, uMsg, wParam, lParam);
    }
    POINT globalPos;
    GetCursorPos(&globalPos);
    RECT rect;
    GetWindowRect(hWnd, &rect);
    if (globalPos.x < rect.left || globalPos.y < rect.top || globalPos.x > rect.right || globalPos.y > rect.bottom) {
        win->onMouseMove(-999999, -999999);
        return CallWindowProc(oldProc, hWnd, uMsg, wParam, lParam);
    }

    HWND hwnd = WindowFromPoint(globalPos);
    WCHAR className[28];
    int len = GetClassName(hwnd, className, 28);
    if ((lstrcmp(TEXT("SysListView32"), className) != 0) && (lstrcmp(TEXT("WorkerW"), className) != 0) && (lstrcmp(TEXT("Progman"), className) != 0)) {
        win->onMouseMove(-999999, -999999);
        return CallWindowProc(oldProc, hWnd, uMsg, wParam, lParam);
    }
    auto raw = Util::getRawInput((HRAWINPUT)lParam);
    if (raw->header.dwType == RIM_TYPEMOUSE)
    {
        RAWMOUSE rawMouse = raw->data.mouse;
        auto x{ globalPos.x - rect.left }, y{ globalPos.y - rect.top };
        if (rawMouse.usButtonFlags == RI_MOUSE_WHEEL)
        {
            auto wheelDelta = (short)rawMouse.usButtonData;
            win->onMouseWheel(x, y, wheelDelta);
            //ListContent::get()->scroll(wheelDelta);
        }
        else {
            switch (rawMouse.ulButtons)
            {
                case RI_MOUSE_LEFT_BUTTON_DOWN:
                {
                    win->onMouseDown(x, y, rawMouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN);
                    break;
                }
                default:
                {
                    win->onMouseMove(x, y);
                    break;
                }
            }
        }
    }
    return CallWindowProc(oldProc, hWnd, uMsg, wParam, lParam);
}