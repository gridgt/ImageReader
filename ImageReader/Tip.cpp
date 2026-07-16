#include "pch.h"
#include "D2D.h"
#include "Tip.h"
#include "WebSocket.h"


Tip::Tip(WindowBase* win, const std::wstring& id) :Node(win,id)
{
    //visual = win->compositor.CreateSpriteVisual();
    //win->rootVisual.Children().InsertAtTop(visual);
    //auto d2d = D2D::get();
    //surface = d2d->createDrawingSurface(win->compositor);
    //Composition::CompositionSurfaceBrush brush = win->compositor.CreateSurfaceBrush(surface);
    //visual.Brush(brush);
}

Tip::~Tip()
{
}

void Tip::show(const float& x, const float& y, const std::wstring& text)
{
    //textLayout.Reset();
    //auto d2d = D2D::get();
    //textLayout = d2d->createTextLayout(text, FLT_MAX, FLT_MAX);
    //textLayout->SetFontSize(14.f * win->dpi, { 0, INT_MAX });
    //DWRITE_TEXT_METRICS metrics;
    //textLayout->GetMetrics(&metrics);

    //visual.Offset({ x, y,0.f });
    //winrt::Windows::Foundation::Numerics::float2 size{ 18.f*win->dpi+metrics.width,  29.f * win->dpi };
    //visual.Size(size);
    //surface.Resize({ (int)size.x, (int)size.y });
    //textPos.x = 9.f*win->dpi;
    //textPos.y = (size.y - metrics.height) / 2;
    //visual.IsVisible(true);
    //paint();
}

void Tip::paint()
{
    //auto [s, d2d] = paintStart(surface);
    //auto skin = Skin::get();
    //auto size = visual.Size();
    //ComPtr<ID2D1SolidColorBrush> brush;
    //d2d->CreateSolidColorBrush(skin->tipInfoBg.getD2DColor(), brush.GetAddressOf());
    //auto r{ 4 * win->dpi };
    //D2D1_ROUNDED_RECT rr = { {0,0,size.x,size.y}, r,r };
    //d2d->FillRoundedRectangle(rr, brush.Get());
    //d2d->CreateSolidColorBrush(skin->tipInfo.getD2DColor(), brush.GetAddressOf());
    //d2d->DrawTextLayout(textPos, textLayout.Get(), brush.Get());
    //s->EndDraw();
}

