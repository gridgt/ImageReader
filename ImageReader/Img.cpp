#include "pch.h"
#include "Img.h"
#include "MainWin.h"
#include "Render.h"

static ComPtr<IWICImagingFactory> imgFactory;

Img::Img(MainWin* win) : win(win)
{

}

Img::~Img()
{

}

void Img::load(const std::wstring& path)
{
    if (!imgFactory.Get()) {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(imgFactory.GetAddressOf())
        );
    }
    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = imgFactory->CreateDecoderFromFilename( path.data(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf());
    if (FAILED(hr)) {
        return;
    }
    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, frame.GetAddressOf());
    if (FAILED(hr)) {
        return;
    }
    ComPtr<IWICFormatConverter> converter;
    hr = imgFactory->CreateFormatConverter(converter.GetAddressOf());
    if (FAILED(hr)) {
        return;
    }
    hr = converter->Initialize(frame.Get(),GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,nullptr,0.0f,WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
        return;
    }
    auto render = win->render->render.Get();
    hr = render->CreateBitmapFromWicBitmap(converter.Get(), nullptr, bitmap.GetAddressOf());
    if (FAILED(hr)) {
        return;
    }
    InvalidateRect(win->hwnd, nullptr, FALSE);
}

void Img::paint()
{
	if (!bitmap.Get()) return;
    auto render = win->render->render.Get();
    // 获取图像原始尺寸
    D2D1_SIZE_F size = bitmap->GetSize();

    // 完整绘制到 (0,0) - (width, height)
    D2D1_RECT_F destRect = D2D1::RectF(0, 0, size.width, size.height);
    render->DrawBitmap(bitmap.Get(), destRect);
    // 或者拉伸到整个窗口：
    // D2D1_RECT_F destRect = D2D1::RectF(0, 0, (float)w, (float)h);
    // renderTarget->DrawBitmap(d2dBitmap, destRect, 1.0f, 
    //     D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}
