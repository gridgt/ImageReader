#include "pch.h"
#include "Util.h"

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
    return false;
}


