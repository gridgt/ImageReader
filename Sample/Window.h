#pragma once
#include "pch.h"

class Page;
class Window
{
public:
	Window(const std::wstring& url);
	~Window();
	static Window* create(const std::wstring& url);
	void show();
	static bool hasWindow(const HWND hwnd);
public:
	HWND hwnd;
private:
	static LRESULT CALLBACK winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void createWin();
	HRESULT onCtrlReady(HRESULT result, ICoreWebView2Controller* ctrl);
	void onSize(int w, int h);
	void onDestroy();
private:
	std::unique_ptr<Page> page;
	ComPtr<ICoreWebView2Controller> ctrl;
	std::wstring url;
};

