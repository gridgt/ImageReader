#include <dwmapi.h>
#include "WinViewer.h"
#include "Message.h"
#include "Environment.h"


WinViewer::WinViewer(winrt::hstring imgPath):imgPath{imgPath}
{
    initPosSize();
    createWindow();
    addShadow();
    show();
    initView(Environment::get()->env.Get());
}

WinViewer::~WinViewer()
{
}

void WinViewer::init(winrt::hstring imgPath)
{
    auto winViewer = new WinViewer(imgPath);
}

void WinViewer::initPosSize()
{
    w = 1000;
    h = 800;
    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMon, &mi);
    RECT work = mi.rcWork;
    int width = work.right - work.left;
    int height = work.bottom - work.top;
    x = work.left + (width - w) / 2;
    y = work.top + (height - h) / 2;
}

void WinViewer::onViewReady()
{
    webview->Navigate(L"https://app.localhost/viewer.html");
}

void WinViewer::addShadow()
{
    MARGINS margins = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    int value = 2;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &value, sizeof(value));
    DwmSetWindowAttribute(hwnd, DWMWA_ALLOW_NCPAINT, &value, sizeof(value));
}

LRESULT WinViewer::procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCHITTEST) {
        return hittest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    else if (msg == WM_GETMINMAXINFO) {
        setMinMaxInfo((LPMINMAXINFO)lParam);
        return 0;
    }
    else if (msg == WM_SIZE) {
        onSize(wParam);
        return 0;
    }
    else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return WinBase::procNativeMsg(msg, wParam, lParam);
}

void WinViewer::procProcMsg(Message* msg)
{
    auto methodName = msg->param.GetNamedString(L"$methodName");
    if (methodName == L"readImg") {
        readImg(msg);
    }
}

void WinViewer::onSize(UINT param)
{
    if (param == SIZE_MAXIMIZED) {
        if (eventTargets.contains(L"win_maximize")) {
            eventTargets[L"win_maximize"]->resolve();
        }
    }
    else if (param == SIZE_RESTORED) {
        if (eventTargets.contains(L"win_restore")) {
            eventTargets[L"win_restore"]->resolve();
        }
    }
    if (webviewCtrl)
    {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        webviewCtrl->put_Bounds(bounds);
    }
}

void WinViewer::setMinMaxInfo(LPMINMAXINFO lpMMI)
{
    MONITORINFO mi = { sizeof(mi) };
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfo(hMonitor, &mi)) {
        lpMMI->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
        lpMMI->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
        lpMMI->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
        lpMMI->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
    }
    lpMMI->ptMinTrackSize.x = 800;
    lpMMI->ptMinTrackSize.y = 600;
}

LRESULT WinViewer::hittest(const int& x, const int& y)
{
    RECT winRect;
    GetWindowRect(hwnd, &winRect);
    if (x > winRect.left && y > winRect.top && x < winRect.right && y < winRect.bottom) {
        static int borderWidth = 5;
        if (x < winRect.left + borderWidth && y < winRect.top + borderWidth) return HTTOPLEFT;
        else if (x < winRect.left + borderWidth && y > winRect.bottom - borderWidth) return HTBOTTOMLEFT;
        else if (x > winRect.right - borderWidth && y > winRect.bottom - borderWidth) return HTBOTTOMRIGHT;
        else if (x > winRect.right - borderWidth && y < winRect.top + borderWidth) return HTTOPRIGHT;
        else if (x < winRect.left + borderWidth) return HTLEFT;
        else if (x > winRect.right - borderWidth) return HTRIGHT;
        else if (y < winRect.top + borderWidth) return HTTOP;
        else if (y > winRect.bottom - borderWidth) return HTBOTTOM;
        else if (x < winRect.right - 100*dpi && y < winRect.top + 30*dpi) {
            return HTCAPTION;
        }
        return HTCLIENT;
    }
    else
    {
        return HTNOWHERE;
    }
}

ComPtr<IStream> WinViewer::procLocalRes(std::wstring& resName)
{
    auto ext = std::filesystem::path(imgPath.data()).extension().wstring();
    resName = L"$$img." + ext;
    ComPtr<IStream> stream;
    HRESULT hr = SHCreateStreamOnFile(imgPath.data(), STGM_READ, stream.GetAddressOf());
    return stream;
}

winrt::Windows::Foundation::IAsyncAction WinViewer::readImg(Message* msg)
{
    co_await winrt::resume_background();
    FILE* fp;
    auto err = _wfopen_s(&fp, imgPath.c_str(), L"rb");
    if (err != 0 || !fp)
    {
        throw std::runtime_error("Failed to open image file.");
    }
    Pix* image = pixReadStream(fp, IFF_DEFAULT);
    fclose(fp);
    auto tess = Environment::get()->tess;
    tess->SetImage(image);
    tess->Recognize(0);
    JsonArray lineArr;
    JsonArray wordArr;
    int lineIndex{ 0 };
    tesseract::ResultIterator* ri = tess->GetIterator();
    do {
        if (ri->IsAtBeginningOf(tesseract::RIL_TEXTLINE)) {
            JsonObject line;
            const char* lineText = ri->GetUTF8Text(tesseract::RIL_TEXTLINE);
            line.SetNamedValue(L"text", JsonValue::CreateStringValue(winrt::to_hstring(lineText)));
            delete[] lineText;
            int lx1, ly1, lx2, ly2;
            ri->BoundingBox(tesseract::RIL_TEXTLINE, &lx1, &ly1, &lx2, &ly2);
            line.SetNamedValue(L"x1", JsonValue::CreateNumberValue(lx1));
            line.SetNamedValue(L"y1", JsonValue::CreateNumberValue(ly1));
            line.SetNamedValue(L"x2", JsonValue::CreateNumberValue(lx2));
            line.SetNamedValue(L"y2", JsonValue::CreateNumberValue(ly2));
            lineArr.Append(line);
            lineIndex += 1;
        }
        const char* wordText = ri->GetUTF8Text(tesseract::RIL_WORD);
        if (wordText) {
            JsonObject word;
            word.SetNamedValue(L"text", JsonValue::CreateStringValue(winrt::to_hstring(wordText)));
            delete[] wordText;
            int wx1, wy1, wx2, wy2;
            ri->BoundingBox(tesseract::RIL_WORD, &wx1, &wy1, &wx2, &wy2);
            word.SetNamedValue(L"x1", JsonValue::CreateNumberValue(wx1));
            word.SetNamedValue(L"y1", JsonValue::CreateNumberValue(wy1));
            word.SetNamedValue(L"x2", JsonValue::CreateNumberValue(wx2));
            word.SetNamedValue(L"y2", JsonValue::CreateNumberValue(wy2));
            word.SetNamedValue(L"lineIndex", JsonValue::CreateNumberValue(lineIndex));
            wordArr.Append(word);
        }
    } while (ri->Next(tesseract::RIL_WORD));
    pixDestroy(&image);
    msg->result.SetNamedValue(L"lines", lineArr);
    msg->result.SetNamedValue(L"words", wordArr);
    Environment::get()->uiDQ.TryEnqueue([msg]()
        {
            msg->resolve();
        });


    //auto file = co_await winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(path.data());
    //auto stream = co_await file.OpenAsync(winrt::Windows::Storage::FileAccessMode::Read);
    //auto decoder = co_await BitmapDecoder::CreateAsync(stream);
    //auto softwareBitmap = co_await decoder.GetSoftwareBitmapAsync();
    ////// 2. 转为灰度（OcrEngine 要求 B8G8R8A8 或 Gray8）
    ////if (softwareBitmap.BitmapPixelFormat() != BitmapPixelFormat::Gray8)
    ////{
    ////    softwareBitmap = SoftwareBitmap::Convert(softwareBitmap, BitmapPixelFormat::Gray8);
    ////}
    //OcrEngine ocrEngine = OcrEngine::TryCreateFromUserProfileLanguages();
    //OcrResult result = co_await ocrEngine.RecognizeAsync(softwareBitmap);
    //std::wstring text;
    //for (auto const& line : result.Lines())
    //{
    //    auto count = line.Words().Size();
    //    for (OcrWord const& word : line.Words())
    //    {
    //        winrt::hstring text = word.Text();
    //        winrt::Windows::Foundation::Rect wordRect = word.BoundingRect();
    //    }
    //    //winrt::Windows::Foundation::Rect lineRect = line.BoundingRect();
    //    //text += line.Text() + L"\n";
    //}
}

