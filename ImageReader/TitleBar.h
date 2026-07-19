#pragma once
#include "pch.h"
#include "Node.h"
class WindowBase;
class TitleBar
{
public:
	~TitleBar();
	static void init(WindowBase* win);
	static TitleBar* get();
private:
	TitleBar(WindowBase* win);
	void onBarSize(void* e);
	void onSize(void* e);
	void onEnter(void* e);
	void onLeave(void* e);
	void onDown(void* e);
	void onBtnPaint(void* e);
	void onPaint(void* e);
	void onMaximize();
	void onRestore();
private:
	Node* node;
	ComPtr<IDWriteTextLayout> title;
	D2D1_POINT_2F titlePos;
	std::vector<Node*> btns;
	std::vector<ComPtr<IDWriteTextLayout>> icons;
	std::vector<D2D1_POINT_2F> poss;
	int hoverIndex{ -1 };
};

