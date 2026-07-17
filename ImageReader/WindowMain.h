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
	void onMouseWheel(const float& x, const float& y, const short& delta) override;
	void onTimer(const UINT& timerId) override;
	void onMinMaxInfo(MINMAXINFO* mmi) override;
	LRESULT onHitTest(const float& x, const float& y) override;
private:
	Composition::CompositionDrawingSurface surface{ nullptr };
};

