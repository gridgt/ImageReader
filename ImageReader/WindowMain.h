#pragma once
#include "WindowBase.h"
class TitleBar;
class TitleBtn;
class WindowMain : public WindowBase
{
public:
	~WindowMain();
	static void init();
	static WindowMain* get();
public:
private:
	WindowMain();
	void onCreated() override;	
	void onMouseWheel(const int& x, const int& y, const short& delta) override;
	void onTimer(const UINT& timerId) override;
	LRESULT onHitTest(const int& x, const int& y) override;
	BOOL setCursor() override;
private:
	Composition::CompositionDrawingSurface surface{ nullptr };
};

