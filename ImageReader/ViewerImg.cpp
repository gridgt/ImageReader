#include "pch.h"
#include "ViewerImg.h"
#include "WindowBase.h"
#include "WindowMain.h"
#include "D2D.h"
#include "Node.h"
#include "tinyocr.h"
#include <iostream>
#include <fstream>
#include <wincodec.h>

static std::unique_ptr<ViewerImg> ins;

ViewerImg::ViewerImg(WindowBase* win, const std::wstring& path)
{
	bitmap = D2D::get()->createBitmap(path);
	win->on("mouseMove", [this](void* e) {this->onMove(e);});
	auto d2d = D2D::get();
	node = win->root->createChild("viewer");
	node->on("sizeChange", [this](void* e) {this->onSize(e);});
	node->on("mouseDown", [this](void* e) {this->onDown(e);});
	node->on("cursor", [this](void* e) {this->onCursor(e);});
	node->on("paint", [this](void* e) {this->onPaint(e);});
	node->initSurface();
	node->sizeChange();//后添加的元素必须自己触发一次
}

ViewerImg::~ViewerImg()
{
}

void ViewerImg::init(WindowBase* win, const std::wstring& path)
{
	if (!ins.get()) {
		ins.reset(new ViewerImg(win,path));
	}
	else {
		ins->bitmap = D2D::get()->createBitmap(path);
		ins->onSize(nullptr);
	}
}
ViewerImg* ViewerImg::get()
{
	return ins.get();
}
void ViewerImg::setPathes(const std::vector<ComPtr<ID2D1PathGeometry>>& pathes)
{
	this->pathes = pathes;
	node->paint();
}
void ViewerImg::onSize(void* e)
{
	auto win = WindowMain::get();
	auto y{ 30.f * win->dpi }, h{ win->h - y - 22 * win->dpi }, w{ win->w };
	node->setPosSize(0.f, y, w, h);
	auto bitmapSize = bitmap->GetSize();
	float bmpW = bitmapSize.width;
	float bmpH = bitmapSize.height;
	scale = 1.0f;
	if (bmpW > w || bmpH > h) {
		float scaleX = w / bmpW;
		float scaleY = h / bmpH;
		scale = std::min(scaleX, scaleY);
	}
	float scaledW = bmpW * scale;
	float scaledH = bmpH * scale;
	pos.x = (w - scaledW) / 2.0f;
	pos.y = (h - scaledH) / 2.0f;

	node->paint();
}

void ViewerImg::onDown(void* e)
{
	isMouseDown = true;
}

void ViewerImg::onMove(void* e)
{
	auto tuplePtr = static_cast<std::tuple<float,float>*>(e);
	auto [x, y] = *tuplePtr;
	auto win = WindowMain::get();
	float nodeY = 30.f * win->dpi;
	auto transform = D2D1::Matrix3x2F::Scale(scale, scale) * D2D1::Matrix3x2F::Translation(pos.x, pos.y + nodeY);
	isHover = false;
	for (auto& path : pathes)
	{
		BOOL contains = FALSE;
		path->FillContainsPoint({ x, y }, transform, &contains);
		if (contains) {
			isHover = true;
			break;
		}
	}
	if (isMouseDown) {

	}
}

void ViewerImg::onPaint(void* e)
{
	auto tuplePtr = static_cast<std::tuple<Node*, ID2D1DeviceContext*>*>(e);
	auto ctx = std::get<1>(*tuplePtr);
	D2D1_RECT_F dstRect = D2D1::RectF(pos.x, pos.y,
		pos.x + bitmap->GetSize().width * scale,
		pos.y + bitmap->GetSize().height * scale
	);
	ctx->DrawBitmap(bitmap.Get(),dstRect,1.0f,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
	if (pathes.empty()) return;
	ComPtr<ID2D1SolidColorBrush> bgBrush;
	ColorA color(0xAA228866);
	ctx->CreateSolidColorBrush(color.getD2DColor(), bgBrush.GetAddressOf());
	D2D1_MATRIX_3X2_F oldTransform;
	ctx->GetTransform(&oldTransform);
	auto transform = D2D1::Matrix3x2F::Scale(scale, scale) * D2D1::Matrix3x2F::Translation(pos.x, pos.y);
	ctx->SetTransform(transform * oldTransform);
	for (auto& path:pathes)
	{
		ctx->FillGeometry(path.Get(), bgBrush.Get());
	}
	ctx->SetTransform(oldTransform);
}

void ViewerImg::onCursor(void* e)
{
	if (isHover) {
		SetCursor(LoadCursor(nullptr, IDC_IBEAM));
		auto flag = static_cast<bool*>(e);
		*flag = true;
	}
}
