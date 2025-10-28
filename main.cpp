#include <Windows.h>
#include "WindowMain.h"
#include "Environment.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    WindowMain::init();
    auto flag = Environment::init();
    if (!flag) return -1;
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    Environment::uninit();
    return 0;
}