#pragma once
#include <include/Ling.h>
class ImgViewer :public Ling::Node
{
	
public:
	ImgViewer(Ling::WinBase* win);
	~ImgViewer();
	void loadImg(const std::wstring& imgPath);
private:
	void paint();
	void layout() override;
private:
	Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap;
	winrt::Windows::UI::Composition::CompositionDrawingSurface surface{ nullptr };
};

