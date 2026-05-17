#pragma once
#include "pch.h"
class MainWin;
class Img
{
public:
	Img(MainWin* win);
	~Img();
	void load(const std::wstring& path);
	void paint();
	void changeCursor(int x, int y);
	void changeSize();
	void onMouseDown(int x, int y);
	void onMouseDrag(int x, int y);
private:
	winrt::fire_and_forget read();
private:
	MainWin* win;
	ComPtr<ID2D1Bitmap> bitmap;
	std::wstring path;
	std::vector<D2D1_RECT_F> ocrRects;
	std::vector<std::wstring> ocrTexts;
	std::vector<D2D1_RECT_F> cursorRects;
	ComPtr<ID2D1SolidColorBrush> highlight;
	long long duration;
	bool isTextCursor{ false };
	float scale;
	D2D1_RECT_F imgRect;
};

