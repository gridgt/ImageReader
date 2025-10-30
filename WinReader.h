#pragma once

#include "WinBase.h"

class WinReader : public WinBase
{
	public:
		WinReader();
		~WinReader();
		static void init();
		static WinReader* get();
	private:
		LRESULT procMsg(UINT msg, WPARAM wParam, LPARAM lParam) override;
		void setMinMaxInfo(LPMINMAXINFO lpMMI);
		void onFileDrop(HDROP hDrop);
		void onSize(UINT param);
		void initPosSize();
		void transparentWindow();
		void onViewReady() override;
	private:
};

