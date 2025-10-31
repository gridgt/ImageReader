#include <dwmapi.h>
#include "WinImgReader.h"
#include "Message.h"
#include "Environment.h"

std::unique_ptr<WinImgReader> instance;

WinImgReader::WinImgReader()
{
    this->tessTask = initTess();
    initPosSize();
    createWindow();
    show();
}

WinImgReader::~WinImgReader()
{
    tess->End();
    delete tess;
}

void WinImgReader::init()
{
    instance = std::make_unique<WinImgReader>();
}

WinImgReader* WinImgReader::get()
{
    return instance.get();
}

bool langReader(const char* filename, std::vector<char>* data)
{
    auto resName = winrt::to_hstring(filename);
    HRSRC hRes = FindResource(NULL, resName.data(), RT_RCDATA);
    if (!hRes) return false;
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return false;
    void* pData = LockResource(hData);
    DWORD size = SizeofResource(NULL, hRes);
    data->resize(size);
    memcpy(data->data(), pData, size);
    return true;
}


winrt::Windows::Foundation::IAsyncAction WinImgReader::initTess()
{
    co_await winrt::resume_background();
    tess = new tesseract::TessBaseAPI();
    tess->Init(nullptr, "eng+chi_sim");
    //int err = tess->Init(nullptr, 0, "eng+chi_sim", tesseract::OEM_DEFAULT, nullptr, 0, nullptr, nullptr, false, &langReader);
}

void WinImgReader::initPosSize()
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

void WinImgReader::onViewReady()
{
    webview->Navigate(L"https://app.localhost/viewer.html");
}

LRESULT WinImgReader::procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_GETMINMAXINFO) {
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

void WinImgReader::procProcMsg(Message* msg)
{
    auto methodName = msg->param.GetNamedString(L"$methodName");
    if (methodName == L"readImg") {
        readImg(msg);
    }
}

void WinImgReader::onSize(UINT param)
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

void WinImgReader::setMinMaxInfo(LPMINMAXINFO lpMMI)
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

ComPtr<IStream> WinImgReader::procLocalRes(std::wstring& resName)
{
    auto ext = std::filesystem::path(imgPath.data()).extension().wstring();
    resName = L"$$img." + ext;
    ComPtr<IStream> stream;
    HRESULT hr = SHCreateStreamOnFile(imgPath.data(), STGM_READ, stream.GetAddressOf());
    return stream;
}

winrt::Windows::Foundation::IAsyncAction WinImgReader::readImg(Message* msg)
{
    if(!tess){ co_await tessTask; }
    co_await winrt::resume_background();
    auto arr = msg->result.GetNamedArray(L"$files");
    imgPath = arr.GetStringAt(0);
    FILE* fp;
    auto err = _wfopen_s(&fp, imgPath.c_str(), L"rb");
    if (err != 0 || !fp)
    {
        throw std::runtime_error("Failed to open image file.");
    }
    Pix* image = pixReadStream(fp, IFF_DEFAULT);
    fclose(fp);
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
    co_await winrt::resume_foreground(Environment::get()->dq);
    msg->resolve();
}

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

