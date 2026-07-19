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
	auto d2d = D2D::get();
	node = win->root->createChild("viewer");
	node->on("sizeChange", [this](void* e) {this->onSize(e);});
	node->on("mouseDown", [this](void* e) {this->onDown(e);});
	node->on("paint", [this](void* e) {this->onPaint(e);});
	node->initSurface();
	bitmap = D2D::get()->createBitmap(path);
	onSize(nullptr);
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
		ins->node->paint();
	}
}
ViewerImg* ViewerImg::get()
{
	return ins.get();
}
void ViewerImg::onSize(void* e)
{
	auto win = WindowMain::get();
	auto y{ 30.f * win->dpi }, h{ win->h - y - 22 * win->dpi };
	node->setPosSize(0.f, y, win->w, h);
	pos.x = 0;
	pos.y = 0;
	node->paint();
}

void ViewerImg::onDown(void* e)
{
}

void ViewerImg::onPaint(void* e)
{
	auto tuplePtr = static_cast<std::tuple<Node*, ID2D1DeviceContext*>*>(e);
	auto ctx = std::get<1>(*tuplePtr);
	ctx->DrawImage(bitmap.Get(), pos, // targetOffset
		D2D1_INTERPOLATION_MODE_LINEAR, // interpolationMode}
		D2D1_COMPOSITE_MODE_SOURCE_OVER // compositeMode
	);
}