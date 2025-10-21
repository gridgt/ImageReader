#include <Windows.h>
#include <tesseract/baseapi.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    //tvg::Initializer::init(std::thread::hardware_concurrency());
    auto hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        MessageBox(NULL, L"Failed to initialize COM library", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    CoUninitialize();
    //tvg::Initializer::term();
    return 0;
}