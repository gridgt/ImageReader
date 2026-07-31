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
	// 算出位图在控件里的最终绘制参数：
	//   scale = 等比缩放系数（不放大；图比控件大时取较小的那一维系数）
	//   dx,dy = 位图左上角在控件内的偏移（保证居中）
	void getDrawParams(float& scale, float& dx, float& dy) const;
	// 在 ctx 上画原图（按 getDrawParams 等比缩放 + 居中）
	void drawBitmap(ID2D1DeviceContext* ctx);
	// 在 ctx 上画 OCR 字符框（用与位图相同的变换；半透明色覆盖）
	void drawRects(ID2D1DeviceContext* ctx, POINT surfaceOffset);
private:
	Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap;
	winrt::Windows::UI::Composition::CompositionDrawingSurface surface{ nullptr };
	// OCR 字符覆盖框用的半透明实心画刷（懒创建）
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> overlayBrush{ nullptr };
	std::vector<Microsoft::WRL::ComPtr<ID2D1PathGeometry>> rects;
};

