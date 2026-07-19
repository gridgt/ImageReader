#pragma once
#include "pch.h"
class D2D
{
public:
	~D2D();
	static D2D* get();
	static ID2D1Factory1* getFactory();
	Composition::CompositionDrawingSurface createDrawingSurface(const Composition::Compositor& comp, const float& w=0, const float& h=0);
	ComPtr<IDWriteTextLayout> createTextLayout(const std::wstring& text, const float& w, const float& h);
	void setEllipsis(IDWriteTextLayout* layout,const float& maxW, const float& maxH);
	ComPtr<ID2D1Bitmap> createBitmap(const std::wstring& imgPath);
private:
	D2D();
	void initFont();
	void initDevice();
	void initIcon();
private:
	ComPtr<IDWriteFactory5> dwriteFactory;
	ComPtr<IDWriteTextFormat> baseTextFormat;
	ComPtr<IDWriteTextFormat> iconFormat;
	ComPtr<ID2D1Device> d2dDevice;
	ComPtr<ID2D1DeviceContext> deviceContext;
	ComPtr<ID2D1Factory1> d2dFactory;
	ComPtr<IDWriteFontCollection1> fontCollection;
};

