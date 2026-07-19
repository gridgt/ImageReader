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

std::wstring Util::convertToWStr(const char* str)
{
    if (!str) return std::wstring();
    int count = MultiByteToWideChar(CP_UTF8, 0, str, -1, 0, 0);
    if (count == 0) return std::wstring();
    std::vector<wchar_t> buffer(count);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, buffer.data(), count);
    return std::wstring(buffer.data(), buffer.size() - 1);
}
