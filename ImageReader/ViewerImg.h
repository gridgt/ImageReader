#pragma once
#include "pch.h"
#include "Node.h"
class WindowBase;
class ViewerImg
{
public:
	~ViewerImg();
	static void init(WindowBase* win,const std::wstring& path);
	static ViewerImg* get();
private:
	ViewerImg(WindowBase* win, const std::wstring& path);
	void onSize(void* e);
	void onDown(void* e);
	void onPaint(void* e);
private:
	D2D1_POINT_2F pos;
	bool isHover{false};
	Node* node;
	ComPtr<ID2D1Bitmap> bitmap;
};

