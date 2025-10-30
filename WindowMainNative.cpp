#include <dwmapi.h>
#include "WindowMain.h"
#include "Message.h"
#include "Environment.h"

std::unique_ptr<WindowMain> winIns;

WindowMain::WindowMain()
{
    createTessAPI();
    createCompCtrl();
    createWindow();
}

WindowMain::~WindowMain()
{
    tessAPI->End();
    delete tessAPI;
}

void WindowMain::init()
{
    auto ins = new WindowMain();
    winIns.reset(ins);
}

WindowMain* WindowMain::get()
{
    return winIns.get();
}

void WindowMain::createTessAPI()
{
    tessAPI = new tesseract::TessBaseAPI();
    tessAPI->Init(nullptr, "eng+chi_sim");
}

void WindowMain::createWindow()
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &WindowMain::winMsg;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_WINLOGO);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"ImageReader";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_WINLOGO);
    RegisterClassExW(&wcex);


    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    int x, y;
    GetMonitorInfo(hMon, &mi);
    RECT work = mi.rcWork;
    int width = work.right - work.left;
    int height = work.bottom - work.top;
    x = work.left + (width - w) / 2;
    y = work.top + (height - h) / 2;
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, wcex.lpszClassName, wcex.lpszClassName, WS_POPUP,
        x, y, w, h, nullptr, nullptr, wcex.hInstance, nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    //MARGINS margins = { 1, 1, 1, 1 };
    //DwmExtendFrameIntoClientArea(hwnd, &margins);
    //int value = 2;
    //DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &value, sizeof(value));
    //DwmSetWindowAttribute(hwnd, DWMWA_ALLOW_NCPAINT, &value, sizeof(value));


    HRGN region = CreateRectRgn(0, 0, -1, -1);
    DWM_BLURBEHIND bb = { 0 };
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = region;
    bb.fEnable = TRUE;
    DwmEnableBlurBehindWindow(hwnd, &bb);
    DeleteObject(region);

    DragAcceptFiles(hwnd, TRUE);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

LRESULT WindowMain::winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto self = reinterpret_cast<WindowMain*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!self) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    if (msg == WM_ERASEBKGND) {
        return 0;
    }
    else if (msg == WM_NCHITTEST) {
        return HTCAPTION;
    }
    else if (msg == WM_DROPFILES) {
        self->onFileDrop((HDROP)wParam);
        return 0;
    }
    else if (msg == WM_GETMINMAXINFO) {
        self->setMinMaxInfo((LPMINMAXINFO)lParam);
        return 0;
    }
    else if (msg == WM_SIZE) {
        self->onSize(wParam);
        return 0;
    }
    else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WindowMain::onFileDrop(HDROP hDrop)
{
    UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
    for (UINT i = 0; i < fileCount; ++i) {
        eventTargets[L"win_reading"]->resolve();
        auto msg = eventTargets[L"win_end"];
        {
            TCHAR filePath[MAX_PATH];
            DragQueryFile(hDrop, i, filePath, MAX_PATH);
            msg->result.SetNamedValue(L"imgPath", JsonValue::CreateStringValue(filePath));
        }
        readImg(msg);
    }
    DragFinish(hDrop);
}

winrt::Windows::Foundation::IAsyncAction WindowMain::readImg(Message* msg)
{
    co_await winrt::resume_background();
    FILE* fp;
    auto path = msg->result.GetNamedString(L"imgPath");
    auto err = _wfopen_s(&fp,path.c_str(), L"rb");
    if (err != 0 || !fp)
    {
        throw std::runtime_error("Failed to open image file.");
    }
    Pix* image = pixReadStream(fp, IFF_DEFAULT);
    fclose(fp);
    tessAPI->SetImage(image);
    tessAPI->Recognize(0);
    JsonArray lineArr;
    JsonArray wordArr;
    int lineIndex{ 0 };
    tesseract::ResultIterator* ri = tessAPI->GetIterator();
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
    msg->resolve();
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

void WindowMain::setMinMaxInfo(LPMINMAXINFO lpMMI)
{
    MONITORINFO mi = { sizeof(mi) };
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfo(hMonitor, &mi)) {
        lpMMI->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
        lpMMI->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
        lpMMI->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
        lpMMI->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
    }
    lpMMI->ptMinTrackSize.x = 600;
    lpMMI->ptMinTrackSize.y = 600;
}

void WindowMain::onSize(UINT param)
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
