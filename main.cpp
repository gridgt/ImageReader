#include <Windows.h>
#include "WindowMain.h"
#include "Environment.h"


#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    WindowMain::init();
    auto flag = Environment::init();
    if (!flag) return -1;



    //tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    //if (api->Init(nullptr, "eng+chi_sim")) {
    //    return 1;
    //}
    //Pix* image = pixRead("test.png");
    //api->SetImage(image);
    //auto outText = api->GetAltoText(0);
    ////auto outText = api->GetUTF8Text();
    //int count = MultiByteToWideChar(CP_UTF8, 0, outText, -1, 0, 0);
    //std::wstring wstr(count, 0);
    //MultiByteToWideChar(CP_UTF8, 0, outText, -1, &wstr[0], count);
    ////std::cout << "OCR Result:\n" << outText << std::endl;
    //delete[] outText;
    //pixDestroy(&image);
    //api->End();
    //delete api;


    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    Environment::uninit();
    return 0;
}