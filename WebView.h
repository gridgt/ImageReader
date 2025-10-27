#pragma once
#include <string>
#include <map>
#include <unordered_set>
#include <functional>

#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
using namespace winrt::Windows::Data::Json;
using namespace Microsoft::WRL;


class WindowBase;
class WebView
{
	public:
		WebView(WindowBase* parentWindow, ICoreWebView2_22* webview);
		~WebView();
	private:		
		HRESULT domLoaded(ICoreWebView2* webview, ICoreWebView2DOMContentLoadedEventArgs* args);
		HRESULT msgReceived(ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args);
		HRESULT resRequested(ICoreWebView2* webview, ICoreWebView2WebResourceRequestedEventArgs* args);
		void addRequestFilter();
		std::wstring getContentType(const std::wstring& fileName);
	private:
		WindowBase* parentWindow;
		ComPtr<ICoreWebView2_22> webview;
		
};

