#pragma once
#include "pch.h"
class Window;
class Page
{
public:
	Page(Window* win, ICoreWebView2* webview);
	~Page();
	void navigate(const std::wstring& url);
private:
	HRESULT onRequest(ICoreWebView2* webview, ICoreWebView2WebResourceRequestedEventArgs* args);
	HRESULT onDomLoaded(ICoreWebView2* sender, ICoreWebView2DOMContentLoadedEventArgs* args);
	std::wstring getContentType(const std::wstring& fileName);
	HRESULT onMsgReceived(ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args);
	void openOneFile();
private:
	Window* win;
	ComPtr<ICoreWebView2> webview;

	
};

