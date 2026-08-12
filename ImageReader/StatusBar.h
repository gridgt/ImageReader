#pragma once
#include <include/Ling.h>
class StatusBar :public Ling::Node
{
public:
	StatusBar(Ling::WinBase* win);
	~StatusBar();
	void setStatusText(const std::wstring& text);
	// 带状态提示与耗时统计地跑一次识别：点按钮选图和命令行传图都走这里
	bool loadImg(const std::wstring& imgPath);
private:
	void onClick();
	std::wstring getFilePath();
private:
	Ling::Label* label;
	Ling::Button* btn;
};

