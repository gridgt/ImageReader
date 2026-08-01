#pragma once
#include <include/Ling.h>
#include <vector>
class ImgViewer :public Ling::Node
{
	
public:
	ImgViewer(Ling::WinBase* win);
	~ImgViewer();
	void loadImg(const std::wstring& imgPath);
private:
	void paint();
	void layout() override;
	void readImg(const uint8_t* data, UINT w, UINT h);
	void getDrawParams(float& scale, float& dx, float& dy) const;
	void drawBitmap(ID2D1DeviceContext* ctx);
	void drawRects(ID2D1DeviceContext* ctx, POINT surfaceOffset);
	void onDown(POINT pt, bool isRight);
	void onMove(POINT pt);
	void onUp(POINT pt);
private:
	Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap;
	winrt::Windows::UI::Composition::CompositionDrawingSurface surface{ nullptr };
	// OCR 字符覆盖框用的半透明实心画刷（懒创建）
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> overlayBrush{ nullptr };
	std::vector<Microsoft::WRL::ComPtr<ID2D1PathGeometry>> rects;
};

