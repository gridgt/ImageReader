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
private:
	TitleBar(WindowBase* win);
	void onSize(const EventArg& e);
	void onEnter(const MouseEventArg& e);
	void onLeave(const EventArg& e);
private:
	Node* btn0;
	Node* btn1;
	Node* btn2;
};

