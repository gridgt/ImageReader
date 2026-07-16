#pragma once
#include "EventArg.h"
class MouseEventArg : public EventArg
{
public:
	MouseEventArg(const int& x,const int& y,bool isRight);
	~MouseEventArg();
public:
	int x, y;
	bool isRight;
};

