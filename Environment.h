#pragma once
#include <string>
#include <windows.h>
#include <WebView2.h>
#include <wrl.h>
#include <functional>
#include <winrt/Windows.System.h>
using namespace Microsoft::WRL;

class Environment
{
	public:
		Environment();
		~Environment();
		static bool init();
		static void uninit();
		static Environment* get();
	public:
		ComPtr<ICoreWebView2Environment> env;
		winrt::Windows::System::DispatcherQueue uiDQ;
	private:
		bool initEnv();
		bool initCOM();
		bool initDataPath();
		bool checkVersion();
		bool checkRegKey(const HKEY& key, const std::wstring& subKey);
		std::wstring dataPath;
};

