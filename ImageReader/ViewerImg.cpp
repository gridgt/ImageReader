#include "pch.h"
#include "ViewerImg.h"
#include "ViewerText.h"
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
	win->on("mouseUp", [this](void* e) {this->onUp(e);});
	auto d2d = D2D::get();
	node = win->root->createChild("viewerImg");
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
void ViewerImg::setPathes(const std::map<int, std::vector<float>>& boxPoints, const std::map<int, std::vector<float>>& charPoints)
{
	pathes.clear();
	charLines.clear();
	selStartBox = selStartChar = selEndBox = selEndChar = -1;
	auto d2d = D2D::get();
	for (size_t i = 0; i < boxPoints.size(); i++)
	{
		auto& boxArr = boxPoints.at(i);
		pathes.push_back(d2d->createPath(boxArr));

		auto& pointArr = charPoints.at(i);
		std::vector<std::pair<D2D1_POINT_2F, D2D1_POINT_2F>> lines;
		auto maxX = std::max({ boxArr[0], boxArr[2], boxArr[4], boxArr[6] });
		auto minX = std::min({ boxArr[0], boxArr[2], boxArr[4], boxArr[6] });
		auto maxY = std::max({ boxArr[1], boxArr[3], boxArr[5], boxArr[7] });
		auto minY = std::min({ boxArr[1], boxArr[3], boxArr[5], boxArr[7] });
		// 头部补一条位于 minX 的线，作为“第 0 个字符的左边界”
		// 之后 lines[j] 表示字符 j 的左边界、lines[j+1] 表示字符 j 的右边界
		lines.push_back({ { minX, minY },{ minX, maxY } });
		if (pointArr.size() > 0) {
			auto maxPoint = pointArr[pointArr.size() - 1];
			auto perVal = (maxX - minX) / maxPoint;
			for (size_t i = 0; i < pointArr.size(); i++)
			{
				auto x = pointArr[i] * perVal + minX;
				lines.push_back({ { x, minY },{ x, maxY } });
			}
		}
		else {
			lines.push_back({ { maxX, minY },{ maxX, maxY } });
		}
		charLines[i] = lines;
	}
	syncSelectionToText();
	node->paint();
}
void ViewerImg::onSize(void* e)
{
	auto win = WindowMain::get();
	auto y{ 30.f * win->dpi }, h{ win->h - y - 22 * win->dpi }, w{ win->w-360.f };
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
	SetCapture(node->win->hwnd);
	isMouseDown = true;
	if (selStartBox >= 0) {
		selStartBox = selStartChar = selEndBox = selEndChar = -1;
		syncSelectionToText();
		node->paint();
	}
	auto tuplePtr = static_cast<std::tuple<float, float, bool, Node*>*>(e);
	auto [x, y, isRight, nodePtr] = *tuplePtr;
	if (isRight) {
		lastDownTick = 0;
		return;
	}
	// 双击：两次左键 mouseDown 时间差 < 系统双击间隔，则选中所有文本
	auto now = GetTickCount64();
	bool isDoubleClick = (lastDownTick != 0) && (now - lastDownTick <= GetDoubleClickTime());
	lastDownTick = isDoubleClick ? 0 : now; // 消费掉这次配对，避免三击继续被视为双击
	if (isDoubleClick && !charLines.empty()) {
		auto first = charLines.begin();
		auto last = std::prev(charLines.end());
		selStartBox = first->first;
		selStartChar = 0;
		selEndBox = last->first;
		selEndChar = static_cast<int>(last->second.size()) - 1;
		isMouseDown = false; // 双击后不进入拖拽扩选模式
		syncSelectionToText();
		node->paint();
		return;
	}
	if (isHover) {
		int boxIdx = -1, charIdx = -1;
		if (hitTest(x, y, boxIdx, charIdx)) {
			selStartBox = boxIdx;
			selStartChar = charIdx;
			selEndBox = boxIdx;
			selEndChar = charIdx;
			syncSelectionToText();
			return;
		}
	}
}

void ViewerImg::onUp(void* e)
{
	isMouseDown = false;
	ReleaseCapture();
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
				syncSelectionToText();
				node->paint();
			}
		}
	}
}

void ViewerImg::onPaint(void* e)
{
	auto tuplePtr = static_cast<std::tuple<Node*, ID2D1DeviceContext*>*>(e);
	auto ctx = std::get<1>(*tuplePtr);
	D2D1_RECT_F dstRect{ pos.x, pos.y, pos.x + bitmap->GetSize().width * scale, pos.y + bitmap->GetSize().height * scale };
	ctx->DrawBitmap(bitmap.Get(),dstRect,1.0f,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
	if (pathes.empty()) return;
	ComPtr<ID2D1SolidColorBrush> selBrush;
	ctx->CreateSolidColorBrush(ColorA(0x66AAFF88).getD2DColor(), selBrush.GetAddressOf());
	D2D1_MATRIX_3X2_F oldTransform;
	ctx->GetTransform(&oldTransform);
	auto transform = D2D1::Matrix3x2F::Scale(scale, scale) * D2D1::Matrix3x2F::Translation(pos.x, pos.y);
	ctx->SetTransform(transform * oldTransform);
	if (selStartBox >= 0 && selEndBox >= 0) {
		int sBox = selStartBox, sChar = selStartChar;
		int eBox = selEndBox, eChar = selEndChar;
		if (sBox > eBox || (sBox == eBox && sChar > eChar)) {
			std::swap(sBox, eBox);
			std::swap(sChar, eChar);
		}
		for (int i = sBox; i <= eBox; i++) {
			auto it = charLines.find(i);
			if (it == charLines.end() || it->second.size() < 2) continue;
			auto& lines = it->second;
			int lastIdx = static_cast<int>(lines.size()) - 1;
			int a = (i == sBox) ? sChar : 0;
			int b = (i == eBox) ? eChar : lastIdx;
			if (a > b) std::swap(a, b);
			a = std::clamp(a, 0, lastIdx);
			b = std::clamp(b, 0, lastIdx);
			// a==b 时，选区退化为一个 caret 位置 —— 涂当前 caret 所指的那个字符 用 lines[a] 与 lines[a+1] 作为该字符的左右边界，末尾时向左取一格
			if (a == b) {
				if (a < lastIdx) b = a + 1;
				else a = b - 1;
			}
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
	// 找到 y 方向最近的 box：优先包含 iy 的行；y 距离相同则用 x 距离决胜
	// （同一行可能有多个横向排布的 box）
	int bestBox = -1;
	float bestYDist = FLT_MAX;
	float bestXDist2 = FLT_MAX;
	for (auto& kv : charLines) {
		if (kv.second.empty()) continue;
		float minY = kv.second.front().first.y;
		float maxY = kv.second.front().second.y;
		if (minY > maxY) std::swap(minY, maxY);
		float yDist;
		if (iy >= minY && iy <= maxY) yDist = 0.f;
		else if (iy < minY) yDist = minY - iy;
		else yDist = iy - maxY;
		float bMinX = kv.second.front().first.x;
		float bMaxX = kv.second.back().first.x;
		if (bMinX > bMaxX) std::swap(bMinX, bMaxX);
		float xDist;
		if (ix >= bMinX && ix <= bMaxX) xDist = 0.f;
		else if (ix < bMinX) xDist = bMinX - ix;
		else xDist = ix - bMaxX;
		if (yDist < bestYDist || (yDist == bestYDist && xDist < bestXDist2)) {
			bestYDist = yDist;
			bestXDist2 = xDist;
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

void ViewerImg::paintAssist(ID2D1DeviceContext* ctx)
{
	ComPtr<ID2D1SolidColorBrush> bgBrush;
	ctx->CreateSolidColorBrush(ColorA(0xAA228822).getD2DColor(), bgBrush.GetAddressOf());
	ComPtr<ID2D1SolidColorBrush> borderBrush;
	ctx->CreateSolidColorBrush(ColorA(0x2288AA88).getD2DColor(), borderBrush.GetAddressOf());
	for (size_t i = 0; i < pathes.size(); i++)
	{
		auto& path = pathes[i];
		ctx->FillGeometry(path.Get(), bgBrush.Get());
		for (size_t j = 0; j < charLines[i].size(); j++)
		{
			auto& pair = charLines[i][j];
			ctx->DrawLine(pair.first, pair.second, borderBrush.Get(), node->win->dpi);
		}
	}
}

void ViewerImg::syncSelectionToText()
{
	auto text = ViewerText::get();
	if (!text) return;
	if (selStartBox < 0 || selEndBox < 0) {
		text->setSelection(-1, -1, -1, -1);
		return;
	}
	// 与 onPaint 相同的规范化 + 单 caret 展开成一个字符的规则，保持视觉一致
	int sBox = selStartBox, sChar = selStartChar;
	int eBox = selEndBox, eChar = selEndChar;
	if (sBox > eBox || (sBox == eBox && sChar > eChar)) {
		std::swap(sBox, eBox);
		std::swap(sChar, eChar);
	}
	if (sBox == eBox && sChar == eChar) {
		auto it = charLines.find(sBox);
		if (it != charLines.end() && it->second.size() >= 2) {
			int lastIdx = static_cast<int>(it->second.size()) - 1;
			if (sChar < lastIdx) eChar = sChar + 1;
			else sChar = eChar - 1;
		}
	}
	text->setSelection(sBox, sChar, eBox, eChar);
}
