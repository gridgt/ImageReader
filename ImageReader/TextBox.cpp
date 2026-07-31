#include "pch.h"
#include "TextBox.h"

TextBox::TextBox(Ling::WinBase* win) : Ling::Node(win)
{
	setHeightPercent(100.f);
	//setBg(0xffff00ff);
}

TextBox::~TextBox()
{

}
