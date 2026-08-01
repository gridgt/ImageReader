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
	void onDown(POINT pt, bool isRight);
	void onMove(POINT pt);
	void onUp(POINT pt);
private:
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush{ nullptr };
	winrt::Windows::UI::Composition::CompositionDrawingSurface surface{ nullptr };
	std::vector<Microsoft::WRL::ComPtr<IDWriteTextLayout>> lineLayouts;
	std::vector<D2D1_POINT_2F> linePoss;
};

