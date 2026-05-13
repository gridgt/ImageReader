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
private:
	MainWin* win;
	ComPtr<ID2D1Bitmap> bitmap;
};

