#include "pch.h"
#include <Windows.UI.Composition.Interop.h> 
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <wincodec.h>
#include "ImgViewer.h"
#include <include/D2D.h>

using namespace Microsoft::WRL;


ImgViewer::ImgViewer(Ling::WinBase* win) :Ling::Node(win)
{
	setHeightPercent(100.f);
	setFlexGrow(1.f);
}

ImgViewer::~ImgViewer()
{
}

void ImgViewer::loadImg(const std::wstring& imgPath)
{
	ComPtr<IWICImagingFactory> wicFactory;
	auto hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
	ComPtr<IWICBitmapDecoder> decoder;
	hr = wicFactory->CreateDecoderFromFilename(imgPath.data(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	ComPtr<IWICBitmapFrameDecode> frame = nullptr;
	hr = decoder->GetFrame(0, &frame);
	ComPtr<IWICFormatConverter> converter = nullptr;
	hr = wicFactory->CreateFormatConverter(&converter);
	hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
	auto d2d = Ling::D2D::get();
	hr = d2d->deviceContext->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &bitmap);
	win->refresh();
}

void ImgViewer::paint()
{
	const int pxW = static_cast<int>(w);
	const int pxH = static_cast<int>(h);
	if (pxW <= 0 || pxH <= 0 || !bitmap) return;
	if (!surface) {
		auto d2d = Ling::D2D::get();
		surface = d2d->createDrawingSurface(win->compositor, (float)pxW, (float)pxH);
		auto brush = win->compositor.CreateSurfaceBrush(surface);
		brush.Stretch(winrt::Windows::UI::Composition::CompositionStretch::None);
		visual.Brush(brush);
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
	D2D1_RECT_F dstRect{ 0.f, 0.f, (float)pxW, (float)pxH };
	ctx->DrawBitmap(bitmap.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
	s->EndDraw();
}

void ImgViewer::layout()
{
	Node::layout();
	paint();
}
