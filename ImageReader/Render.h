#pragma once
#include "pch.h"
class MainWin;
class Btn;
class Render
{
public:
	Render(MainWin* win);
	~Render();
	void changeSize();
	void paint();
public:
	ComPtr<ID2D1HwndRenderTarget> render;
	ComPtr<IDWriteFactory5> dwriteFactory;
	D2D1_RECT_F headerRect, bodyRect;
private:
	void init();
	void initRes();
	bool isInRect(int x, int y, D2D1_RECT_F& rect);
private:
	ComPtr<ID2D1Factory> d2d;
	ComPtr<IDWriteTextFormat> textFormat;
	ComPtr<ID2D1SolidColorBrush> headerBg;
	ComPtr<ID2D1SolidColorBrush> bodyBg;
	ComPtr<ID2D1SolidColorBrush> textBrush;
	ComPtr<IDWriteTextLayout> headerTextLayout;
	float headerTextX{ 10 }, headerTextY;
	std::wstring headerText{ L"拖拽文件到窗口内，识别图像中的文字" };
	MainWin* win;
};

