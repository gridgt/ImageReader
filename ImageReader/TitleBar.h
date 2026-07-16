#pragma once
#include "pch.h"
#include "Node.h"
class WindowBase;
class TitleBar
{
public:
	~TitleBar();
	static void init(WindowBase* win);
private:
	TitleBar(WindowBase* win);
private:
	Node* btn0;
	Node* btn1;
	Node* btn2;
};

