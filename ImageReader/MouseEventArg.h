#pragma once
#include "EventArg.h"
class MouseEventArg : public EventArg
{
public:
	MouseEventArg(const float& x, const float& y, bool isRight);
	~MouseEventArg();
public:
	float x, y;      // 逻辑像素（DIPs）
	bool isRight;
};

