#include "pch.h"
#include "Render.h"
#include "MainWin.h"
#include "Btn.h"
#include "Img.h"

Render::Render(MainWin* win):win{win}
{
    init();
    initRes();
}

Render::~Render()
{}

void Render::changeSize()
{
    render->Resize(D2D1::SizeU(win->w, win->h));
    auto headerHeight = win->dpi * win->headerHeight;
    headerRect = D2D1::RectF(0, 0, win->w, headerHeight);
    bodyRect = D2D1::RectF(0, headerHeight, win->w, win->h);

    headerTextLayout->SetFontSize(14 * win->dpi, { 0, static_cast<UINT32>(headerText.length()) });
    DWRITE_TEXT_METRICS textMetrics;
    headerTextLayout->GetMetrics(&textMetrics);
    headerTextY = (headerHeight - textMetrics.height) / 2.0f;
    headerTextX = 10 * win->dpi;

    win->btnClose->changeSize();
	win->btnMax->changeSize();
    win->btnRestore->changeSize();
    win->btnMini->changeSize();

}

void Render::init()
{
    auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2d.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Create D2D1Factory error");
    }
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(dwriteFactory.GetAddressOf()));
    if (FAILED(hr)) {
        throw std::runtime_error("dwriteFactory create error");
    }
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT, //D2D1_RENDER_TARGET_TYPE_DEFAULT,//D2D1_RENDER_TARGET_TYPE_HARDWARE,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96, 96
    );
    D2D1_SIZE_U size = D2D1::SizeU(win->w, win->h);
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(win->hwnd, size);
    hr = d2d->CreateHwndRenderTarget(rtProps, hwndProps, render.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("CreateHwndRenderTarget error");
    }
}

void Render::initRes()
{
    dwriteFactory->CreateTextFormat(L"Microsoft YaHei", nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        15, L"", textFormat.GetAddressOf());

    render->CreateSolidColorBrush(D2D1::ColorF(0xA7C6DB, 0.68), headerBg.GetAddressOf());
    render->CreateSolidColorBrush(D2D1::ColorF(0xF6F6F6, 0.68), bodyBg.GetAddressOf());
    render->CreateSolidColorBrush(D2D1::ColorF(0x333333, 1.f), textBrush.GetAddressOf());
    
    dwriteFactory->CreateTextLayout(headerText.data(), headerText.size(), textFormat.Get(), FLT_MAX, FLT_MAX, &headerTextLayout);
    headerTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    headerTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

bool Render::isInRect(int x, int y, D2D1_RECT_F& rect)
{
    if (x > rect.left && x<rect.right && y>rect.top && y < rect.bottom) return true;
    return false;
}

void Render::paint()
{
	render->BeginDraw();
	render->Clear();
	render->FillRectangle(headerRect, headerBg.Get());
	render->FillRectangle(bodyRect, bodyBg.Get());
    render->DrawTextLayout({ headerTextX, headerTextY }, headerTextLayout.Get(), textBrush.Get(), D2D1_DRAW_TEXT_OPTIONS_NONE);
    win->btnClose->paint();
	win->btnMax->paint();
	win->btnRestore->paint();
    win->btnMini->paint();
	win->img->paint();
	render->EndDraw();
}