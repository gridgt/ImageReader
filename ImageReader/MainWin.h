#pragma once
#include "pch.h"
class Render;
class Btn;
class Img;
class MainWin
{
public:
	MainWin();
	~MainWin();
	static void init();
public:
	HWND hwnd;
	int x, y, w, h;
	float dpi, headerHeight{40};
	std::unique_ptr<Render> render;
	std::unique_ptr<Img> img;
	std::unique_ptr<Btn> btnMini, btnMax, btnRestore, btnClose;
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void createWin();
	void setShadow();
	void setPos();
	void enableAlpha();
	void show();
	void onPaint();
	void onSize(int w, int h);
	void onMaximize();
	void onRestore();
	void onMinMaxInfo(MINMAXINFO* mmi);
	void onMouseMove(int x, int y);
	void onClick();
	void onMouseLeave();
	void onDpiChange(int dpi);
	LRESULT onHitTest(const POINT& pt);
	void onDestroy();
	void onDropFiles(HDROP hDrop);
private:
	bool isMouseTracking{ false };
};

