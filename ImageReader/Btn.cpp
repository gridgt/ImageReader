#include "pch.h"
#include "Btn.h"
#include "MainWin.h"
#include "Render.h"


static ComPtr<IDWriteTextFormat> format;

Btn::Btn(const std::wstring& icon, int index, MainWin* win)
	: icon(icon), index(index), win(win)
{
    if(!format.Get()) initFormat();
	auto render = win->render->render.Get();
    if (index == 0) {
        render->CreateSolidColorBrush(D2D1::ColorF(0xED4C4C, 0.86f), bg.GetAddressOf());
        render->CreateSolidColorBrush(D2D1::ColorF(0xFFFFFF, 1.0f), colorHover.GetAddressOf());
    }
    else {
        render->CreateSolidColorBrush(D2D1::ColorF(0x9DBACE, 0.86f), bg.GetAddressOf());
        render->CreateSolidColorBrush(D2D1::ColorF(0x222222, 1.0f), colorHover.GetAddressOf());
    }
    render->CreateSolidColorBrush(D2D1::ColorF(0x666666, 1.0f), color.GetAddressOf());
    auto dwriteFactory = win->render->dwriteFactory.Get();
    dwriteFactory->CreateTextLayout(icon.data(), icon.size(), format.Get(), FLT_MAX, FLT_MAX, &layout);
    layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

Btn::~Btn()
{

}

void Btn::paint()
{
    if (!isVisible) return;
	auto render = win->render->render.Get();
	if (isHover) {
		render->FillRectangle(rect, bg.Get());
	}
	render->DrawTextLayout(
        { rect.left + x, rect.top + y }, 
        layout.Get(), 
        isHover ? colorHover.Get() : color.Get(), 
        D2D1_DRAW_TEXT_OPTIONS_NONE
    );
}

void Btn::changeSize()
{
	auto headerHeight = win->dpi * win->headerHeight;
	auto btnWidth{ headerHeight * 1.2 };
	rect = D2D1::RectF(win->w - btnWidth*(index+1), 0, win->w - btnWidth*index, headerHeight);

    DWRITE_TEXT_METRICS textMetrics;
    layout->SetFontSize(14 * win->dpi, { 0, static_cast<UINT32>(icon.length()) });
    layout->GetMetrics(&textMetrics);
    y = (headerHeight - textMetrics.height) / 2.0f;
    x = ((rect.right - rect.left) - textMetrics.width) / 2.0f;
}

void Btn::hitTest(int x, int y)
{
    if (!isVisible) return;
	auto isHover = x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
    if (this->isHover != isHover) {
        this->isHover = isHover;
        InvalidateRect(win->hwnd, nullptr, FALSE);
    }
}

void Btn::initFormat()
{
    HRSRC hRes = FindResource(NULL, L"iconfont.ttf", RT_RCDATA);
    if (!hRes) return;
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return;
    void* pData = LockResource(hData);
    DWORD size = SizeofResource(NULL, hRes);
    ComPtr<IDWriteInMemoryFontFileLoader> loader;
	auto dwriteFactory = win->render->dwriteFactory.Get();
    dwriteFactory->CreateInMemoryFontFileLoader(loader.GetAddressOf());
    dwriteFactory->RegisterFontFileLoader(loader.Get());
    ComPtr<IDWriteFontFile> fontFile;
    loader->CreateInMemoryFontFileReference(dwriteFactory, pData, size, nullptr, fontFile.GetAddressOf());
    ComPtr<IDWriteFontSetBuilder1> fontSetBuilder;
    dwriteFactory->CreateFontSetBuilder(fontSetBuilder.GetAddressOf());
    fontSetBuilder->AddFontFile(fontFile.Get());
    ComPtr<IDWriteFontSet> fontSet;
    fontSetBuilder->CreateFontSet(fontSet.GetAddressOf());
    ComPtr<IDWriteFontCollection1> fontCollection;
    dwriteFactory->CreateFontCollectionFromFontSet(fontSet.Get(), fontCollection.GetAddressOf());
    dwriteFactory->CreateTextFormat(L"icon", fontCollection.Get(),
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        12.f, L"", format.GetAddressOf());
}
