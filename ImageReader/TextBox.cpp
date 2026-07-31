#include "pch.h"
#include "TextBox.h"
#include <Windows.UI.Composition.Interop.h> 

using namespace Microsoft::WRL;

TextBox::TextBox(Ling::WinBase* win) : Ling::Node(win)
{
	setHeightPercent(100.f);

}

TextBox::~TextBox()
{

}

void TextBox::loadText(const std::vector<std::wstring>& texts)
{

}

void TextBox::paint()
{
	const int pxW = static_cast<int>(w);
	const int pxH = static_cast<int>(h);
	if (pxW <= 0 || pxH <= 0 ) return;
	if (!surface) {
		auto d2d = Ling::D2D::get();
		surface = d2d->createDrawingSurface(win->compositor, (float)pxW, (float)pxH);
		auto surfaceBrush = win->compositor.CreateSurfaceBrush(surface);
		surfaceBrush.Stretch(winrt::Windows::UI::Composition::CompositionStretch::None);
		visual.Brush(surfaceBrush);
	}
	else {
		auto sz = surface.SizeInt32();
		if (sz.Width != pxW || sz.Height != pxH) {
			surface.Resize({ pxW, pxH });
		}
	}
	auto s = surface.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>();
	ComPtr<ID2D1DeviceContext> ctx;
	POINT offset{};
	s->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), reinterpret_cast<void**>(ctx.GetAddressOf()), &offset);
	ctx->SetTransform(D2D1::Matrix3x2F::Translation((float)offset.x, (float)offset.y));
	ctx->Clear(0);
	s->EndDraw();
}

void TextBox::layout()
{
	Node::layout();
	paint();
}