#include "pch.h"
#include "MouseEventArg.h"

MouseEventArg::MouseEventArg(const int& x, const int& y, bool isRight) : EventArg(), x{ x }, y{ y }, isRight{isRight}
{
}

MouseEventArg::~MouseEventArg()
{
}
