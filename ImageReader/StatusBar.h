#pragma once
#include <include/Ling.h>
class StatusBar :public Ling::Node
{
public:
	StatusBar(Ling::WinBase* win);
	~StatusBar();
private:
	void onClick();
	std::wstring getFilePath();
private:
	Ling::Label* label;
	Ling::Button* btn;
};

