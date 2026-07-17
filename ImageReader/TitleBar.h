#pragma once
#include "pch.h"
#include "Node.h"
#include "EventArg.h"
#include "MouseEventArg.h"
class WindowBase;
class TitleBar
{
public:
	~TitleBar();
	static void init(WindowBase* win);
	static TitleBar* get();
private:
	TitleBar(WindowBase* win);
	void onBarSize(const EventArg& e);
	void onSize(const EventArg& e);
	void onEnter(void* e);
	void onLeave(const EventArg& e);
	void onDown(const MouseEventArg& e);
	void paint(Node* btn);
	void paintBar();
private:
	Node* bar;
	ComPtr<IDWriteTextLayout> title;
	D2D1_POINT_2F titlePos;
	std::vector<Node*> btns;
	std::vector<ComPtr<IDWriteTextLayout>> icons;
	std::vector<D2D1_POINT_2F> poss;
	int hoverIndex{ -1 };
};

