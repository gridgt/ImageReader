#include "pch.h"
#include "OCR/OcrLite.h"
#include "OCR/OcrUtils.h"
#include "Env.h"
#include "Img.h"
#include "MainWin.h"
#include "Render.h"

static ComPtr<IWICImagingFactory> imgFactory;

Img::Img(MainWin* win) : win(win)
{
    auto render = win->render->render.Get();
    render->CreateSolidColorBrush(D2D1::ColorF(0x11960D, 0.18), highlight.GetAddressOf());
}

Img::~Img()
{

}

void Img::load(const std::wstring& path)
{
    HRESULT hr;
    if (!imgFactory.Get()) {
        hr = CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(imgFactory.GetAddressOf()));
        if (FAILED(hr)) {
            return;
        }
    }
    bitmap.Reset();
    ComPtr<IWICBitmapDecoder> decoder;
    hr = imgFactory->CreateDecoderFromFilename( path.data(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf());
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
	this->path = path;
    read();
    InvalidateRect(win->hwnd, nullptr, FALSE);
}

void Img::paint()
{
    if (!bitmap.Get()) return;
    auto render = win->render->render.Get();
    D2D1_RECT_F& rect = win->render->bodyRect;
    D2D1_SIZE_F size = bitmap->GetSize();
    float availW = rect.right - rect.left;
    float availH = rect.bottom - rect.top;
    scale = (std::min)(1.0f, (std::min)(availW / size.width, availH / size.height));
    float destW = size.width * scale;
    float destH = size.height * scale;
    float offsetX = (availW - destW) * 0.5f;
    float offsetY = (availH - destH) * 0.5f;
    imgRect = D2D1::RectF(
        rect.left + offsetX, rect.top + offsetY,
        rect.left + offsetX + destW, rect.top + offsetY + destH
    );
    render->DrawBitmap(bitmap.Get(), imgRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

void Img::changeCursor(int x, int y)
{
    bool isHover = false;
    for (size_t i = 0; i < cursorRects.size(); i++)
    {
        auto& rect = cursorRects[i];
        if (x > rect.left && x < rect.right && y > rect.top && y < rect.bottom)
        {
            isHover = true;
            break;
        }
    }
    if (isTextCursor != isHover) {
        isTextCursor = isHover;
        if (isTextCursor) {
            SetClassLongPtr(win->hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(nullptr, IDC_IBEAM));
        }
        else {
			SetClassLongPtr(win->hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(nullptr, IDC_ARROW));
        }
	}
}

void Img::changeSize()
{
    cursorRects.clear();
    for (auto& rect : ocrRects)
    {
        float x1 = imgRect.left + rect.left * scale;
        float y1 = imgRect.top + rect.top * scale;
        float x2 = imgRect.left + rect.right * scale;
        float y2 = imgRect.top + rect.bottom * scale;
        auto textRect = D2D1::RectF(x1, y1, x2, y2);
        cursorRects.push_back(std::move(textRect));
    }
}

void Img::onMouseDown(int x, int y)
{
    std::wstring text = L"Hello 世界";
    auto factory = win->render->dwriteFactory.Get();
    ComPtr<IDWriteTextLayout> layout;
    factory->CreateTextLayout(
        text.c_str(),
        (UINT32)text.size(),
        format.Get(),
        1000,
        1000,
        &layout);

    for (UINT32 i = 0; i < text.size(); i++) {

        FLOAT x, y;

        DWRITE_HIT_TEST_METRICS hit{};

        layout->HitTestTextPosition(
            i,
            FALSE,
            &x,
            &y,
            &hit);
        wprintf(
            L"char=%c x=%f width=%f\n",
            text[i],
            x,
            hit.width);
    }
}

void Img::onMouseDrag(int x, int y)
{

}

winrt::fire_and_forget Img::read()
{
    co_await winrt::resume_background();
    ocrRects.clear();
	ocrTexts.clear();
    auto startTime = std::chrono::high_resolution_clock::now();
    std::filesystem::path path(this->path);
    std::wstring dirPath = path.parent_path().wstring() + L"\\";
    std::wstring fileName = path.filename().wstring();
    auto dirStr = Env::convertToStr(dirPath);
    auto nameStr = Env::convertToStr(fileName);
    auto ocr = Env::getOcr();
    OcrResult ocrResult = ocr->detect(dirStr.c_str(), nameStr.c_str(), 50, 1024,
        0.5f, 0.3f, 1.6f, true, true);
    for (auto& block : ocrResult.textBlocks)
    {
        ocrRects.push_back(D2D1::RectF(
            block.boxPoint[0].x, block.boxPoint[0].y,
            block.boxPoint[2].x, block.boxPoint[2].y
		));
		ocrTexts.push_back(Env::convertToWStr(block.text.data()));
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    changeSize();
    co_await winrt::resume_foreground(Env::getDQ());
}
