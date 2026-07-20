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
	node->on("mouseUp", [this](void* e) {this->onUp(e);});
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
void ViewerImg::setPathes(const std::map<int, std::vector<float>>& boxPoints, const std::map<int, std::vector<float>>& charPoints)
{
	pathes.clear();
	charLines.clear();
	clearSelection();
	auto d2d = D2D::get();
	for (size_t i = 0; i < boxPoints.size(); i++)
	{
		auto& boxArr = boxPoints.at(i);
		pathes.push_back(d2d->createPath(boxArr));

		auto& pointArr = charPoints.at(i);
		auto maxPoint = pointArr[pointArr.size() - 1];
		auto maxX = std::max({ boxArr[0], boxArr[2], boxArr[4], boxArr[6] });
		auto minX = std::min({ boxArr[0], boxArr[2], boxArr[4], boxArr[6] });
		auto maxY = std::max({ boxArr[1], boxArr[3], boxArr[5], boxArr[7] });
		auto minY = std::min({ boxArr[1], boxArr[3], boxArr[5], boxArr[7] });
		auto perVal = (maxX - minX) / maxPoint;
		std::vector<std::pair<D2D1_POINT_2F, D2D1_POINT_2F>> lines;
		// 头部补一条位于 minX 的线，作为“第 0 个字符的左边界”
		// 之后 lines[j] 表示字符 j 的左边界、lines[j+1] 表示字符 j 的右边界
		lines.push_back({ { minX, minY },{ minX, maxY } });
		for (size_t i = 0; i < pointArr.size(); i++)
		{
			auto x = pointArr[i] * perVal+minX;
			lines.push_back({ { x, minY },{ x, maxY } });
		}
		charLines[i] = lines;
	}
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
	auto tuplePtr = static_cast<std::tuple<float, float, bool, Node*>*>(e);
	auto [x, y, isRight, nodePtr] = *tuplePtr;
	if (isRight) {
		return;
	}
	// 只在按下位置命中某个 path（文本框）时才启动选区
	auto nodePos = node->visual.Offset();
	auto transform = D2D1::Matrix3x2F::Scale(scale, scale) * D2D1::Matrix3x2F::Translation(pos.x, pos.y + nodePos.y);
	bool onText = false;
	for (auto& path : pathes) {
		BOOL contains = FALSE;
		path->FillContainsPoint({ x, y }, transform, &contains);
		if (contains) { onText = true; break; }
	}
	bool hadSelection = (selStartBox >= 0);
	if (onText) {
		int boxIdx = -1, charIdx = -1;
		if (hitTest(x, y, boxIdx, charIdx)) {
			selStartBox = boxIdx;
			selStartChar = charIdx;
			selEndBox = boxIdx;
			selEndChar = charIdx;
			node->paint();
			return;
		}
	}
	if (hadSelection) {
		clearSelection();
		node->paint();
	}
}

void ViewerImg::onUp(void* e)
{
	isMouseDown = false;
}

void ViewerImg::onMove(void* e)
{
	auto tuplePtr = static_cast<std::tuple<float,float>*>(e);
	auto [x, y] = *tuplePtr;
	auto win = WindowMain::get();
	auto nodePos = node->visual.Offset();
	auto transform = D2D1::Matrix3x2F::Scale(scale, scale) * D2D1::Matrix3x2F::Translation(pos.x, pos.y + nodePos.y);
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
	ColorA color(0xAA228822);
	ctx->CreateSolidColorBrush(color.getD2DColor(), bgBrush.GetAddressOf());
	ComPtr<ID2D1SolidColorBrush> borderBrush;
	ColorA color2(0x2288AA88);
	ctx->CreateSolidColorBrush(color2.getD2DColor(), borderBrush.GetAddressOf());
	ComPtr<ID2D1SolidColorBrush> selBrush;
	ColorA selColor(0x66AAFF88); // 半透明蓝色，选中高亮 (RRGGBBAA)
	ctx->CreateSolidColorBrush(selColor.getD2DColor(), selBrush.GetAddressOf());
	D2D1_MATRIX_3X2_F oldTransform;
	ctx->GetTransform(&oldTransform);
	auto transform = D2D1::Matrix3x2F::Scale(scale, scale) * D2D1::Matrix3x2F::Translation(pos.x, pos.y);
	ctx->SetTransform(transform * oldTransform);
	//for (size_t i = 0; i < pathes.size(); i++)
	//{
	//	auto& path = pathes[i];
	//	ctx->FillGeometry(path.Get(), bgBrush.Get());
	//	for (size_t j = 0; j < charLines[i].size(); j++)
	//	{
	//		auto& pair = charLines[i][j];
	//		ctx->DrawLine(pair.first, pair.second, borderBrush.Get(), node->win->dpi);
	//	}
	//}
	// 绘制选中区域
	if (selStartBox >= 0 && selEndBox >= 0) {
		int sBox = selStartBox, sChar = selStartChar;
		int eBox = selEndBox, eChar = selEndChar;
		if (sBox > eBox || (sBox == eBox && sChar > eChar)) {
			std::swap(sBox, eBox);
			std::swap(sChar, eChar);
		}
		for (int i = sBox; i <= eBox; i++) {
			auto it = charLines.find(i);
			if (it == charLines.end() || it->second.empty()) continue;
			auto& lines = it->second;
			int lastIdx = static_cast<int>(lines.size()) - 1;
			int a = (i == sBox) ? sChar : 0;
			int b = (i == eBox) ? eChar : lastIdx;
			if (a > b) std::swap(a, b);
			a = std::clamp(a, 0, lastIdx);
			b = std::clamp(b, 0, lastIdx);
			if (a == b) continue; // 没有字符被覆盖
			float x0 = lines[a].first.x;
			float x1 = lines[b].first.x;
			float y0 = lines[a].first.y;
			float y1 = lines[a].second.y;
			D2D1_RECT_F r = D2D1::RectF(std::min(x0, x1), std::min(y0, y1),
				std::max(x0, x1), std::max(y0, y1));
			ctx->FillRectangle(r, selBrush.Get());
		}
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

bool ViewerImg::hitTest(float wx, float wy, int& boxIdx, int& charIdx)
{
	if (charLines.empty()) return false;
	auto nodePos = node->visual.Offset();
	// 窗口坐标 -> 图像坐标
	float ix = (wx - pos.x) / scale;
	float iy = (wy - pos.y - nodePos.y) / scale;
	// 找到 y 方向最近的 box：优先包含 iy 的行；否则按到中心距离取最近
	int bestBox = -1;
	float bestDist = FLT_MAX;
	for (auto& kv : charLines) {
		if (kv.second.empty()) continue;
		float minY = kv.second.front().first.y;
		float maxY = kv.second.front().second.y;
		if (minY > maxY) std::swap(minY, maxY);
		float dist;
		if (iy >= minY && iy <= maxY) dist = 0.f;
		else if (iy < minY) dist = minY - iy;
		else dist = iy - maxY;
		if (dist < bestDist) {
			bestDist = dist;
			bestBox = kv.first;
		}
	}
	if (bestBox < 0) return false;
	auto& lines = charLines[bestBox];
	// 在该 box 内找 x 最近的分隔线索引
	int bestIdx = 0;
	float bestXDist = FLT_MAX;
	for (size_t j = 0; j < lines.size(); j++) {
		float dx = std::abs(lines[j].first.x - ix);
		if (dx < bestXDist) {
			bestXDist = dx;
			bestIdx = static_cast<int>(j);
		}
	}
	// 越界时把索引钳制到端点（拖出行左侧 -> 0；行右侧 -> 末尾）
	if (ix < lines.front().first.x) bestIdx = 0;
	else if (ix > lines.back().first.x) bestIdx = static_cast<int>(lines.size()) - 1;
	boxIdx = bestBox;
	charIdx = bestIdx;
	return true;
}

void ViewerImg::clearSelection()
{
	selStartBox = selStartChar = selEndBox = selEndChar = -1;
}
