#include "pch.h"
#include "Util.h"

HWND Util::getWorkerW()
{
    static HWND workerW;
    if (!workerW) {
        HWND progman = FindWindow(L"Progman", NULL);
        SendMessage(progman, 0x052C, 0xD, 0);
        SendMessage(progman, 0x052C, 0xD, 1);
        //if (isWin11()) {
            workerW = FindWindowEx(progman, NULL, L"WorkerW", NULL);
        //}
        if (!workerW) {
            EnumWindows([](HWND hwnd, LPARAM lParam) {
                HWND defView = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
                if (defView != NULL) {
                    auto tar = (HWND*)lParam;
                    *tar = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
                }
                return TRUE;
                }, (LPARAM)&workerW);
        }
    }
    return workerW;
}

bool Util::isWin11()
{
    RTL_OSVERSIONINFOW osInfo = { 0 };
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
    if (!hNtDll) return false;
    typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtDll, "RtlGetVersion");
    if (RtlGetVersion == NULL) return false;
    if (RtlGetVersion(&osInfo) != 0) return false;
    if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion == 0) {
        if (osInfo.dwBuildNumber >= 22000) {
            return true;
        }
        else
        {
            return false;
        }
    }
}
RAWINPUT* Util::getRawInput(HRAWINPUT lParam) {
    UINT dwSize = sizeof(RAWINPUT);
    static BYTE lpb[sizeof(RAWINPUT)];
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
    return (RAWINPUT*)lpb;
}

void Util::regInputDevice(HWND hwnd)
{
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = hwnd? RIDEV_INPUTSINK: RIDEV_REMOVE;
    rid.hwndTarget = hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}
