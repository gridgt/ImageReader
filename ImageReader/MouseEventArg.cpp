#include "pch.h"
#include "MouseEventArg.h"

MouseEventArg::MouseEventArg(const float& x, const float& y, bool isRight) : EventArg(), x{ x }, y{ y }, isRight{ isRight }
{
}

MouseEventArg::~MouseEventArg()
{
}
