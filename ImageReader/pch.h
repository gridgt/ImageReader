#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <wrl.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite_3.h>
#include <dwmapi.h>
#include <wincodec.h>

#include <filesystem>
#include <string>
#include <format>
#include <unordered_map>
#include <unordered_set>

#include <dispatcherqueue.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.AccessCache.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Foundation.Collections.h>
using namespace winrt::Windows::Data::Json;
using namespace winrt::Windows::Storage;
using namespace Microsoft::WRL;
namespace wf = winrt::Windows::Foundation;



template<typename... Args>
void log(std::wstring_view fmt, Args&&... args)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &time);
    std::wstringstream ss;
    ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");
    auto timeStr = ss.str();
    auto msg = std::vformat(fmt, std::make_wformat_args(args...));
    std::wstring final = L"[" + timeStr + L"] " + msg + L"\n";
    OutputDebugString(final.c_str());
}

