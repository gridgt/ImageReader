#pragma once
#include "WinBase.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

class WinImgReader : public WinBase
{
public:
	WinImgReader();
	~WinImgReader();
	static void init();
	static WinImgReader* get();
private:
	winrt::Windows::Foundation::IAsyncAction initTess();
	void initPosSize();
	LRESULT procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam) override;
	ComPtr<IStream> procLocalRes(std::wstring& resName) override;
	void procProcMsg(Message* msg) override;
	void setMinMaxInfo(LPMINMAXINFO lpMMI);
	void onSize(UINT param);
	winrt::Windows::Foundation::IAsyncAction readImg(Message* msg);
	void onViewReady() override;
private:
	tesseract::TessBaseAPI* tess;
	winrt::Windows::Foundation::IAsyncAction tessTask;
	winrt::hstring imgPath;
};

