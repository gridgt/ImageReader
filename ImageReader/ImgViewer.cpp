#include "pch.h"
#include <Windows.UI.Composition.Interop.h> 
#include <TinyOCR/include/tinyocr/tiny_ocr.h>
#include <wincodec.h>
#include "ImgViewer.h"
#include "WindowMain.h"
#include "TextBox.h"

using namespace Microsoft::WRL;


ImgViewer::ImgViewer(Ling::WinBase* win) :Ling::Node(win)
{
	setHeightPercent(100.f);
	setFlexGrow(1.f);
	win->onMouseDown.add([this](POINT pt, bool isRight) {this->onDown(pt, isRight);});
	win->onMouseMove.add([this](POINT pt) {this->onMove(pt);});
	win->onMouseUp.add([this](POINT pt, bool isRight) {this->onUp(pt);});
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

	UINT w = 0, h = 0;
	converter->GetSize(&w, &h);
	std::vector<uint8_t> bgra(static_cast<size_t>(w) * h * 4);
	converter->CopyPixels(nullptr, w * 4, static_cast<UINT>(bgra.size()), bgra.data());

	readImg(bgra.data(), w, h);
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
	drawBitmap(ctx.Get());
	drawRects(ctx.Get(), offset);
	s->EndDraw();
}

void ImgViewer::getDrawParams(float& scale, float& dx, float& dy) const
{
	// 默认值：bitmap 异常时退化为不缩放、不偏移
	scale = 1.f;
	dx = 0.f;
	dy = 0.f;
	if (!bitmap) return;

	const D2D1_SIZE_F bmpSize = bitmap->GetSize();
	const float imgW = bmpSize.width;
	const float imgH = bmpSize.height;
	if (imgW <= 0.f || imgH <= 0.f) return;

	// 控件尺寸与位图都是物理像素（Yoga 已乘过 dpi，bitmap 也是物理像素），可直接比
	const float vw = w;
	const float vh = h;
	// contain: 取较小那一维的缩放系数，保证两边都塞得进
	scale = (vw / imgW < vh / imgH) ? (vw / imgW) : (vh / imgH);
	if (scale > 1.f) scale = 1.f;  // 不放大 —— 图比控件小时保持原尺寸

	const float drawW = imgW * scale;
	const float drawH = imgH * scale;
	dx = (vw - drawW) * 0.5f;
	dy = (vh - drawH) * 0.5f;
}

void ImgViewer::drawBitmap(ID2D1DeviceContext* ctx)
{
	if (!ctx || !bitmap) return;
	float scale, dx, dy;
	getDrawParams(scale, dx, dy);
	const D2D1_SIZE_F bmpSize = bitmap->GetSize();
	const float drawW = bmpSize.width * scale;
	const float drawH = bmpSize.height * scale;
	// 把位图直接 DrawBitmap 到 (dx, dy) ~ (dx+drawW, dy+drawH) —— 缩放由 D2D 内插完成
	D2D1_RECT_F dstRect{ dx, dy, dx + drawW, dy + drawH };
	ctx->DrawBitmap(bitmap.Get(), dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

void ImgViewer::drawRects(ID2D1DeviceContext* ctx, POINT surfaceOffset)
{
	if (!ctx || rects.empty() || !bitmap) return;

	// 字符框的坐标原图坐标系（与 bitmap 同尺度），要画到 surface 上需要套上同样的 「等比缩放 + 居中」变换；再加上 BeginDraw 给的 surface tile offset。
	float scale, dx, dy;
	getDrawParams(scale, dx, dy);

	D2D1_MATRIX_3X2_F oldTransform;
	ctx->GetTransform(&oldTransform);
	auto imgTransform = D2D1::Matrix3x2F::Translation(dx, dy) * D2D1::Matrix3x2F::Scale(scale, scale);
	auto surfaceT = D2D1::Matrix3x2F::Translation((float)surfaceOffset.x, (float)surfaceOffset.y);
	ctx->SetTransform(surfaceT * imgTransform);

	// 懒创建半透明画刷（淡绿色，alpha 0.35）—— 设备相关资源，第一次 paint 时建一次即可
	if (!overlayBrush) {
		ctx->CreateSolidColorBrush(Ling::Color(0x597ef766).getD2DColor(), overlayBrush.GetAddressOf());
	}
	for (auto& path : rects) {
		if (path) ctx->FillGeometry(path.Get(), overlayBrush.Get());
	}

	// 还原 transform，避免影响后续绘制
	ctx->SetTransform(oldTransform);
}

void ImgViewer::onDown(POINT pt, bool isRight)
{
}

void ImgViewer::onMove(POINT pt)
{
}

void ImgViewer::onUp(POINT pt)
{
}

void ImgViewer::layout()
{
	Node::layout();
	paint();
}

void ImgViewer::readImg(const uint8_t* data, UINT w, UINT h)
{
	std::vector<uint8_t> bgr(static_cast<size_t>(w) * h * 3);
	auto dst = bgr.data();
	const size_t n = static_cast<size_t>(w) * h;
	for (size_t i = 0; i < n; ++i) {
		dst[0] = data[0];
		dst[1] = data[1];
		dst[2] = data[2];
		data += 4;
		dst += 3;
	}
	auto [recData, recSize] = Ling::Util::getRes(L"PP-OCRv6_rec_tiny.onnx");
	auto [detData, detSize] = Ling::Util::getRes(L"PP-OCRv6_det_tiny.onnx");

	tinyocr::Options opts;
	opts.det_model_data = detData;
	opts.det_model_size = detSize;
	opts.rec_model_data = recData;
	opts.rec_model_size = recSize;
	opts.return_word_box = true;
	opts.return_single_char_box = true;

	tinyocr::TinyOcr ocr(opts);
	auto result = ocr.run(bgr.data(), w, h, 3);
	auto d2d = Ling::D2D::get();
	std::vector<std::wstring> lines;
	for (size_t i = 0; i < result.lines.size(); ++i) {
		const auto& L = result.lines[i];
		auto text = Ling::Util::convertToWStr(L.text.c_str());
		lines.push_back(text);
		for (size_t j = 0; j < L.words.size(); ++j) {
			const auto& W = L.words[j];
			ComPtr<ID2D1PathGeometry> path;
			d2d->d2dFactory->CreatePathGeometry(path.GetAddressOf());
			ComPtr<ID2D1GeometrySink> sink;
			path->Open(sink.GetAddressOf());
			sink->BeginFigure({ W.box[0].x, W.box[0].y }, D2D1_FIGURE_BEGIN_FILLED);
			sink->AddLine({ W.box[1].x, W.box[1].y });
			sink->AddLine({ W.box[2].x, W.box[2].y });
			sink->AddLine({ W.box[3].x, W.box[3].y });
			sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			sink->Close();
			rects.push_back(path);
		}
	}
	auto cur = dynamic_cast<WindowMain*>(win);
	cur->textBox->loadText(lines);
}
