#include "pch.h"
#include "TextBox.h"
#include <Windows.UI.Composition.Interop.h> 

using namespace Microsoft::WRL;

TextBox::TextBox(Ling::WinBase* win) : Ling::Node(win)
{
	setWidthPercent(100.f);
	setPadding(12.f);
	win->onMouseDown.add([this](POINT pt, bool isRight) {this->onDown(pt, isRight);});
	win->onMouseMove.add([this](POINT pt) {this->onMove(pt);});
	win->onMouseUp.add([this](POINT pt, bool isRight) {this->onUp(pt);});
}

TextBox::~TextBox()
{

}

void TextBox::loadText(const std::vector<std::wstring>& lines)
{
	lineLayouts.clear();
	linePoss.clear();
	auto d2d = Ling::D2D::get();
	auto textW = w - getPaddingLeft() * win->dpi*2;
	auto curHeight{ getPaddingLeft() * win->dpi };
	for (const auto& text : lines) {
		Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
		//设置最大宽度，允许自动换行
		d2d->dwriteFactory->CreateTextLayout(text.data(), (UINT32)text.length(), d2d->baseTextFormat.Get(), textW, FLT_MAX, textLayout.ReleaseAndGetAddressOf());
		textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
		textLayout->SetFontSize(14.f * win->dpi, { 0, INT_MAX });
		DWRITE_TEXT_METRICS metrics;
		textLayout->GetMetrics(&metrics);
		lineLayouts.push_back(std::move(textLayout));
		linePoss.push_back({ 12.f * win->dpi,curHeight });
		curHeight += metrics.height;
	}
	setHeight(curHeight / win->dpi + getPaddingBottom() * 2.f);
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
	if (!textBrush) {
		ctx->CreateSolidColorBrush(Ling::Color(0x333333FF).getD2DColor(), &textBrush);
	}
	ctx->SetTransform(D2D1::Matrix3x2F::Translation((float)offset.x, (float)offset.y));
	ctx->Clear(0);
	auto index = 0;
	for (const auto& lineLayout : lineLayouts) {
		ctx->DrawTextLayout(linePoss[index], lineLayout.Get(), textBrush.Get());
		index++;
	}
	s->EndDraw();
}

void TextBox::layout()
{
	Node::layout();
	for (const auto& lineLayout : lineLayouts) {
		lineLayout->SetMaxWidth(w - getPaddingLeft() * win->dpi * 2);
	}
	paint();
}

void TextBox::onDown(POINT pt, bool isRight)
{
}

void TextBox::onMove(POINT pt)
{
}

void TextBox::onUp(POINT pt)
{
}
