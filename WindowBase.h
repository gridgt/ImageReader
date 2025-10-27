#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <memory>

#include <windows.h>
#include <dwmapi.h>
#include <wrl.h>
#include <WebView2.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
using namespace winrt::Windows::Data::Json;
using namespace Microsoft::WRL;
#define MSG_BACK_ID WM_APP + 10

class WebView;
class Message;
class WindowBase
{
	public:
		WindowBase();
		~WindowBase();
		void hittest(Message* msg);
		void minimize(Message* msg);
		void maximize(Message* msg);
		void close(Message* msg);
		void restore(Message* msg);
		void show(Message* msg);
		virtual void exec(Message* msg);
		void on(Message* msg);
		void off(Message* msg);
	public:
		HWND hwnd;
	protected:
		void createWindow();
		void createShadow();
		virtual LRESULT procMsg(UINT msg, WPARAM wParam, LPARAM lParam);
	protected:
		int x, y, w, h;
		float dpr;
		std::unordered_map<winrt::hstring, std::vector<Message*>> eventTargets;
		ComPtr<ICoreWebView2Controller> webviewCtrl;
	private:
		static LRESULT CALLBACK winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		HRESULT ctrlReady(HRESULT result, ICoreWebView2Controller* env);
	private:
		std::unique_ptr<WebView> webview;
};

