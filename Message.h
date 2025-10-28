#pragma once
#include <functional>
#include <unordered_map>
#include <variant>

#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
using namespace winrt::Windows::Data::Json;
using namespace Microsoft::WRL;

class WindowMain;
class Message
{
	public:
		Message(JsonObject&& param, WindowMain* win);
		~Message();
		void route();
		void resolve();
		void postMsgBack();
	public:
		JsonObject param; 
		JsonObject result;
		WindowMain* win;
	private:
		void initResult();
	private:
		DWORD mainThreadId;
};

