#pragma once
#include "pch.h"
#include "Node.h"
class WindowBase;
class Tip :public Node
{
public:
	Tip(WindowBase* win, const std::string& id);
	~Tip();
	void show(const float& x, const float& y, const std::wstring& text);
private:
	void paint();
private:
	Composition::CompositionDrawingSurface surface{ nullptr };
	ComPtr<IDWriteTextLayout> textLayout;
	D2D1_POINT_2F textPos;
};

