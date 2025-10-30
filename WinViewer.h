#pragma once
#include "WinBase.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

class WinViewer : public WinBase
{
public:
	WinViewer(winrt::hstring imgPath);
	~WinViewer();
	static void init(winrt::hstring imgPath);
private:
	void createTessAPI();
	void initPosSize();
	void addShadow();
	LRESULT procMsg(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void setMinMaxInfo(LPMINMAXINFO lpMMI);
	void onSize(UINT param);
	winrt::Windows::Foundation::IAsyncAction readImg(Message* msg);
	void onViewReady() override;
	void hittest();
private:
	tesseract::TessBaseAPI* tessAPI;
	winrt::hstring imgPath;
};

