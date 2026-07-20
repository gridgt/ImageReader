#include "pch.h"
#include "D2D.h"
#include "Node.h"
#include "NodeScroll.h"
#include "WindowBase.h"

NodeScroll::NodeScroll(WindowBase* win, const std::string& id) :Node(win,id)
{
    visual.Clip(win->compositor.CreateInsetClip());
    // content用于承载子元素，高度为item高度之和，content改变offset y时，子元素被裁剪
    visualContent = win->compositor.CreateSpriteVisual();
    visual.Children().InsertAtTop(visualContent);
    // 滚动条
    visualScroller = win->compositor.CreateSpriteVisual();
    visual.Children().InsertAtTop(visualScroller);
    visualScroller.IsVisible(false);
    win->on("mouseWheel", [this](void* e) {this->onWheel(e);});
    win->on("mouseMove", [this](void* e) {this->onMove(e);});
    win->on("mouseUp", [this](void* e) {this->onUp(e);});
    on("mouseDown", [this](void* e) {this->onDown(e);});
}

NodeScroll::~NodeScroll()
{
}

void NodeScroll::setContentPosSize(const float& x, const float& y, const float& w, const float& h)
{
    visualContent.Offset({ x,y,0.f });
    visualContent.Size({ w,h });
    auto sbW{ 6 * win->dpi };
    auto size = visual.Size();
    visualScroller.Offset({ size.x - sbW, 0.f, 0.f });
    visualScroller.Size({ sbW, size.y });
    surface.Resize({ static_cast<int>(w), static_cast<int>(h) });
    surfaceScroller.Resize({ static_cast<int>(sbW), static_cast<int>(size.y) });
    visualScroller.IsVisible(h > size.y);
}

void NodeScroll::initContentSurface()
{
    auto d2d = D2D::get();
    surface = d2d->createDrawingSurface(win->compositor);
    Composition::CompositionSurfaceBrush brush = win->compositor.CreateSurfaceBrush(surface);
    visualContent.Brush(brush);

    surfaceScroller = d2d->createDrawingSurface(win->compositor);
    Composition::CompositionSurfaceBrush brush2 = win->compositor.CreateSurfaceBrush(surfaceScroller);
    visualScroller.Brush(brush2);
}

void NodeScroll::onWheel(void* e)
{
    auto tuplePtr = static_cast<std::tuple<POINT, short>*>(e);
    auto [pos, delta] = *tuplePtr;
    if (!hasScroller()) return;
    if (!isPosIn(pos.x,pos.y)) return;
    float step = 60.f * win->dpi;
    setScroll(scrollY - (delta / (float)WHEEL_DELTA) * step);
}

void NodeScroll::onDown(void* e)
{
    auto tuplePtr = static_cast<std::tuple<float, float, bool, Node*>*>(e);
    auto y = std::get<1>(*tuplePtr);
    auto isRight = std::get<2>(*tuplePtr);
    if (isRight || !visual.IsVisible()) return;
    if (isHoverScroller) {
        SetCapture(win->hwnd);
        scrollerDragging = true;
        dragStartMouseY = (float)y;
        dragStartScrollY = scrollY;
        return;
    }
}

void NodeScroll::onUp(void* e)
{
    ReleaseCapture();
    scrollerDragging = false;
}

void NodeScroll::onMove(void* e)
{
    if (!visual.IsVisible()) return;
    auto tuplePtr = static_cast<std::tuple<float, float>*>(e);
    auto [x, y] = *tuplePtr;
    if (scrollerDragging) {
        auto contentSize = visualContent.Size();
        auto size = visual.Size();
        float maxScroll = std::max(0.f, contentSize.y - size.y);
        float minH = 24.f * win->dpi;
        float th = std::max(minH, size.y * size.y / contentSize.y);
        float trackFree = size.y - th;
        if (trackFree <= 0) return;
        float ratio = (y - dragStartMouseY) / trackFree;
        setScroll(dragStartScrollY + ratio * maxScroll);
    }
    else {
        auto pos = visualScroller.Offset();
        auto size = visualScroller.Size();
        if (x > absX + pos.x && x<absX + pos.x + size.x && y>absY && y < absY + size.y) {
            if (!isHoverScroller) {
                isHoverScroller = true;
                paintScrollbar();
            }
        }
        else {
            if (isHoverScroller) {
                isHoverScroller = false;
                paintScrollbar();
            }
        }
    }
}

bool NodeScroll::hasScroller()
{
    if(!visual.IsVisible()) return false;
    auto contentSize = visualContent.Size();
    auto size = visual.Size();
    if (size.y >= contentSize.y) return false;
    return true;
}

void NodeScroll::setScroll(float y)
{
    auto contentSize = visualContent.Size();
    auto size = visual.Size();
    float maxScroll = std::max(0.f, contentSize.y - size.y);
    y = std::clamp(y, 0.f, maxScroll); //将一个值限制在指定的上下界范围内
    scrollY = y;
    visualContent.Offset({ 0.f, -scrollY, 0.f });
    if (contentSize.y > size.y) {
        paintScrollbar();
    }
}
void NodeScroll::paintScrollbar()
{
    auto s = surfaceScroller.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>();
    ComPtr<ID2D1DeviceContext> ctx;
    POINT offset{};
    HRESULT hr = s->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), reinterpret_cast<void**>(ctx.GetAddressOf()), &offset);
    auto trans = D2D1::Matrix3x2F::Translation(static_cast<float>(offset.x), static_cast<float>(offset.y));
    ctx->SetTransform(trans);
    ctx->Clear(0);

    ComPtr<ID2D1SolidColorBrush> b;
    D2D1::ColorF color(isHoverScroller ? 0xCCCCCC : 0xDDDDDD);
    ctx->CreateSolidColorBrush(color, b.GetAddressOf());

    auto contentSize = visualContent.Size();
    auto size = visual.Size();
    float minH = 24.f * win->dpi;
    float th = std::max(minH, size.y * size.y / contentSize.y);
    float maxScroll = contentSize.y - size.y;
    float top = maxScroll > 0 ? scrollY * (size.y - th) / maxScroll : 0.f;

    //float radius{ 2.f * win->dpi };
    auto sbW{ 6 * win->dpi };
    D2D1_RECT_F rr{ 0.f, top, sbW, top + th };
    ctx->FillRectangle(rr, b.Get());
    s->EndDraw();
}