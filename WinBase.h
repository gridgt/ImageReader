#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <memory>

#include <filesystem>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <WebView2.h>
#include <DispatcherQueue.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>

using namespace winrt::Windows::Data::Json;
using namespace Microsoft::WRL;

class WebView;
class Message;
class WinBase
{
	public:
		WinBase();
		~WinBase();
		void initView(ICoreWebView2Environment* env);
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
		ComPtr<ICoreWebView2_22> webview;
	protected:
		void show();
		void createWindow();
		virtual LRESULT procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam);
		virtual void procProcMsg(Message* msg) {};
		virtual void onViewReady() {};
		virtual ComPtr<IStream> procLocalRes(std::wstring& resName) { return nullptr; };
	protected:
		int x, y,w, h;
		float dpi;
		std::unordered_map<winrt::hstring, Message*> eventTargets;
		ComPtr<ICoreWebView2Controller> webviewCtrl;
	private:
		static LRESULT CALLBACK winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);	
		void setMinMaxInfo(LPMINMAXINFO lpMMI);
		void onSize(UINT param);
		void addRequestFilter();
		void addMsgReceiver();
		void addDomLoader();
		JsonObject getParam(ICoreWebView2WebMessageReceivedEventArgs* args);
		void getFiles(ICoreWebView2WebMessageReceivedEventArgs* args, Message* msg);
		HRESULT resRequested(ICoreWebView2* webview, ICoreWebView2WebResourceRequestedEventArgs* args);
		std::wstring getContentType(const std::wstring& fileName);
	private:
};

