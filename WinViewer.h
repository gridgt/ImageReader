#pragma once
#include "WinBase.h"

class WinViewer : public WinBase
{
public:
	WinViewer(winrt::hstring imgPath);
	~WinViewer();
	static void init(winrt::hstring imgPath);
private:
	void initPosSize();
	void addShadow();
	LRESULT procNativeMsg(UINT msg, WPARAM wParam, LPARAM lParam) override;
	ComPtr<IStream> procLocalRes(std::wstring& resName) override;
	void procProcMsg(Message* msg) override;
	void setMinMaxInfo(LPMINMAXINFO lpMMI);
	void onSize(UINT param);
	winrt::Windows::Foundation::IAsyncAction readImg(Message* msg);
	void onViewReady() override;
	LRESULT hittest(const int& x,const int& y);
private:
	winrt::hstring imgPath;
};

