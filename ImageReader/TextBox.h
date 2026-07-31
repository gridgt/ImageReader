#pragma once
#include <include/Ling.h>
class TextBox :public Ling::Node
{
public:
	TextBox(Ling::WinBase* win);
	~TextBox();
	void loadText(const std::vector<std::wstring>& texts);
private:
	void paint();
	void layout() override;
private:
	winrt::Windows::UI::Composition::CompositionDrawingSurface surface{ nullptr };
};

