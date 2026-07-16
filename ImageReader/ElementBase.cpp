#include "pch.h"
#include "ElementBase.h"
#include "WindowCalendar.h"

ElementBase::ElementBase(WindowCalendar* win, const std::wstring& id) :win{ win }, id{id}
{
}

ElementBase::~ElementBase()
{
}

void ElementBase::hide()
{
    visual.IsVisible(false);
}

void ElementBase::show()
{
    visual.IsVisible(true);
}

bool ElementBase::isVisible()
{
    return visual.IsVisible();
}

std::pair<winrt::impl::com_ref<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>, ComPtr<ID2D1DeviceContext>> ElementBase::paintStart(Composition::CompositionDrawingSurface& surface)
{
    auto surfaceInterop = surface.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>();
    ComPtr<ID2D1DeviceContext> d2d;
    POINT offset{};
    HRESULT hr = surfaceInterop->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), reinterpret_cast<void**>(d2d.GetAddressOf()), &offset);
    auto trans = D2D1::Matrix3x2F::Translation(static_cast<float>(offset.x), static_cast<float>(offset.y));
    d2d->SetTransform(trans);
    d2d->Clear(0);
    return { surfaceInterop ,d2d };
}
