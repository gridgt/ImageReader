#pragma once
#include "WinBase.h"

class WinImgReader : public WinBase
{
public:
	WinImgReader();
	~WinImgReader();
	static void init();
	static WinImgReader* get();
private:
	void initPosSize();
	LRESULT procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam) override;
	ComPtr<IStream> procLocalRes(std::wstring& resName) override;
	void procProcMsg(Message* msg) override;
	void setMinMaxInfo(LPMINMAXINFO lpMMI);
	void onSize(UINT param);
	winrt::Windows::Foundation::IAsyncAction readImg(Message* msg);
	void onViewReady() override;
	winrt::Windows::Foundation::IAsyncAction initOCR();
private:
	winrt::hstring imgPath;
	winrt::Windows::Foundation::IAsyncAction ocrTask;
};

