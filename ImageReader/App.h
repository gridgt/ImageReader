#pragma once
#include "pch.h"
class App
{
public:
	~App();
	static void init();
	static App* get();
	/// <summary>
	/// winrt::Windows::Foundation::IAsyncAction WinImgReader::readImg(Message* msg)
	/// co_await winrt::resume_background();
	/// co_await winrt::resume_foreground(App::get()->dq);
	/// </summary>
	winrt::Windows::System::DispatcherQueue dq;
private:
	static void initDispatcherQueueCtrl();
private:
	App();
};

