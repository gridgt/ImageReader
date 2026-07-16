#pragma once
#include "pch.h"
#include "ElementBase.h"
class WindowCalendar;
class Tip :public ElementBase
{
public:
	Tip(WindowCalendar* win, const std::wstring& id);
	~Tip();
	void show(const float& x, const float& y, const std::wstring& text);
private:
	void paint();
private:
	Composition::CompositionDrawingSurface surface{ nullptr };
	ComPtr<IDWriteTextLayout> textLayout;
	D2D1_POINT_2F textPos;
};

