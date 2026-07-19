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
	void setPathes(const std::vector<ComPtr<ID2D1PathGeometry>>& pathes);
private:
	ViewerImg(WindowBase* win, const std::wstring& path);
	void onSize(void* e);
	void onDown(void* e);
	void onMove(void* e);
	void onPaint(void* e);
	void onCursor(void* e);
private:
	D2D1_POINT_2F pos;
	float scale{1.0f};
	bool isHover{false};
	Node* node;
	ComPtr<ID2D1Bitmap> bitmap;
	std::vector<ComPtr<ID2D1PathGeometry>> pathes;
};

