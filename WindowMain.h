#pragma once
#include "WindowBase.h"

class WindowMain :public WindowBase
{
public:
	WindowMain();
	~WindowMain();
	static void init();
public:
protected:
	LRESULT procMsg(UINT msg, WPARAM wParam, LPARAM lParam) override;
private:
	void setMinMaxInfo(LPMINMAXINFO lpMMI);
	void onSize(UINT param);
private:
};

