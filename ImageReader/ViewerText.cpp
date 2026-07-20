#include "pch.h"
#include "ViewerText.h"
#include "WindowBase.h"
#include "WindowMain.h"
#include "D2D.h"
#include "Node.h"
#include "tinyocr.h"
#include <iostream>
#include <fstream>
#include <wincodec.h>

static std::unique_ptr<ViewerText> ins;

ViewerText::ViewerText(WindowBase* win)
{
	win->on("mouseMove", [this](void* e) {this->onMove(e);});
	auto d2d = D2D::get();
	node = win->root->createChild("viewerText");
	node->on("sizeChange", [this](void* e) {this->onSize(e);});
	node->on("mouseDown", [this](void* e) {this->onDown(e);});
	node->on("mouseUp", [this](void* e) {this->onUp(e);});
	node->on("cursor", [this](void* e) {this->onCursor(e);});
	node->on("paint", [this](void* e) {this->onPaint(e);});
	node->initSurface();
	node->sizeChange();//后添加的元素必须自己触发一次
}

ViewerText::~ViewerText()
{
}

void ViewerText::init(WindowBase* win)
{
	if (!ins.get()) {
		ins.reset(new ViewerText(win));
	}
	else {
		ins->onSize(nullptr);
	}
}
ViewerText* ViewerText::get()
{
	return ins.get();
}
void ViewerText::setText(const std::vector<std::wstring>& texts)
{
	textLayouts.clear();
	auto size = node->visual.Size();
	auto padding = 12.f * node->win->dpi;
	auto curHeight{ 8.f * node->win->dpi };
	for (size_t i = 0; i < texts.size(); i++)
	{
		auto& str = texts[i];
		auto d2d = D2D::get();
		auto layout = d2d->createTextLayout(str, size.x-padding*2, FLT_MAX);
		layout->SetFontSize(12.f * node->win->dpi ,{0,MAXINT});
		layout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
		DWRITE_TEXT_METRICS metrics;
		layout->GetMetrics(&metrics);
		textLayouts.push_back(layout);
		D2D1_POINT_2F pos{ padding,curHeight };
		textPoss.push_back(std::move(pos));
		curHeight += metrics.height; //行间距
	}
	selStartBox = selStartChar = selEndBox = selEndChar = -1;
	node->paint();
}
void ViewerText::onSize(void* e)
{
	auto win = WindowMain::get();
	auto w{ 360.f },x{win->w-w}, y{ 30.f * win->dpi }, h{ win->h - y - 22 * win->dpi };
	node->setPosSize(x, y, w, h);
	node->paint();
}

void ViewerText::onDown(void* e)
{
	isMouseDown = true;
	if (selStartBox >= 0) {
		selStartBox = selStartChar = selEndBox = selEndChar = -1;
		node->paint();
	}
	auto tuplePtr = static_cast<std::tuple<float, float, bool, Node*>*>(e);
	auto [x, y, isRight, nodePtr] = *tuplePtr;
	if (isRight) {
		return;
	}
	if (isHover) {
		int boxIdx = -1, charIdx = -1;
		if (hitTest(x, y, boxIdx, charIdx)) {
			selStartBox = boxIdx;
			selStartChar = charIdx;
			selEndBox = boxIdx;
			selEndChar = charIdx;
			return;
		}
	}
}

void ViewerText::onUp(void* e)
{
	isMouseDown = false;
}

void ViewerText::onMove(void* e)
{
	auto tuplePtr = static_cast<std::tuple<float,float>*>(e);
	auto [x, y] = *tuplePtr;
	auto win = WindowMain::get();
	auto nodePos = node->visual.Offset();
	if (isMouseDown && selStartBox >= 0) {
		int boxIdx = -1, charIdx = -1;
		if (hitTest(x, y, boxIdx, charIdx)) {
			if (boxIdx != selEndBox || charIdx != selEndChar) {
				selEndBox = boxIdx;
				selEndChar = charIdx;
				node->paint();
			}
		}
	}
}

void ViewerText::onPaint(void* e)
{
	auto tuplePtr = static_cast<std::tuple<Node*, ID2D1DeviceContext*>*>(e);
	auto ctx = std::get<1>(*tuplePtr);

	ComPtr<ID2D1SolidColorBrush> bgBrush;
	ctx->CreateSolidColorBrush(ColorA(0xAA228822).getD2DColor(), bgBrush.GetAddressOf());

	ComPtr<ID2D1SolidColorBrush> brush;
	ctx->CreateSolidColorBrush(ColorA(0x333333FF).getD2DColor(), brush.GetAddressOf());

	ComPtr<ID2D1SolidColorBrush> borderBrush;
	ctx->CreateSolidColorBrush(ColorA(0xDDDDDDFF).getD2DColor(), borderBrush.GetAddressOf());

	auto size = node->visual.Size();
	ctx->DrawLine({ 0.f,0.f }, { 0.f,size.y }, borderBrush.Get(), node->win->dpi*1.5);
	for (size_t i = 0; i < textLayouts.size(); i++)
	{
		auto& layout = textLayouts[i];
		ctx->DrawTextLayout(textPoss[i], layout.Get(), brush.Get());
	}
}

void ViewerText::onCursor(void* e)
{
	SetCursor(LoadCursor(nullptr, IDC_IBEAM));
	auto flag = static_cast<bool*>(e);
	*flag = true;
}

bool ViewerText::hitTest(float wx, float wy, int& boxIdx, int& charIdx)
{
	return true;
}
