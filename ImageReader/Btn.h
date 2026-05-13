#pragma once
#include "pch.h"

class MainWin;
class Btn
{
public:
	Btn(const std::wstring& icon, int index, MainWin* win);
	~Btn();
	void paint();
	void changeSize();
	void hitTest(int x, int y);
public:
	std::wstring icon;
	int index{ 0 };
	bool isHover;
	bool isVisible{true};
	D2D1_RECT_F rect;
private:
	void initFormat();
private:
	ComPtr<ID2D1SolidColorBrush> bg;
	ComPtr<IDWriteTextLayout> layout;
	ComPtr<ID2D1SolidColorBrush> color;
	ComPtr<ID2D1SolidColorBrush> colorHover;
	float x, y;
	MainWin* win;
};

