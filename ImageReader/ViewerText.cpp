#include "pch.h"
#include "ViewerText.h"
#include "ViewerImg.h"
#include "WindowBase.h"
#include "WindowMain.h"
#include "D2D.h"
#include "Node.h"
#include "Util.h"
#include "tinyocr.h"
#include <iostream>
#include <fstream>
#include <wincodec.h>

static std::unique_ptr<ViewerText> ins;

ViewerText::ViewerText(WindowBase* win)
{
	win->on("mouseMove", [this](void* e) {this->onMove(e);});
	win->on("mouseUp", [this](void* e) {this->onUp(e);});
	win->on("keyDown", [this](void* e) {this->onKey(e);});
	win->on("cursor", [this](void* e) {this->onCursor(e);});
	node = win->root->createChildScroller("viewerText");
	node->on("sizeChange", [this](void* e) {this->onSize(e);});
	node->on("mouseDown", [this](void* e) {this->onDown(e);});
	node->on("paint", [this](void* e) {this->onPaint(e);});
	node->initContentSurface();
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
	textPoss.clear();
	textLens.clear();
	textHeights.clear();
	this->texts = texts;
	auto size = node->visual.Size();
	auto padding = 12.f * node->win->dpi;
	auto curHeight{ 8.f * node->win->dpi };
	log(L"[setText] node.visual.Size=({},{}) padding={} layoutWidth={}",
		size.x, size.y, padding, size.x - padding * 2);
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
		textLens.push_back(static_cast<UINT32>(str.size()));
		textHeights.push_back(metrics.height);
		curHeight += metrics.height; //行间距
	}
	selStartBox = selStartChar = selEndBox = selEndChar = -1;
	syncSelectionToImg();
	node->setContentPosSize(0,0,size.x,curHeight);
	node->paint();
}
void ViewerText::setSelection(int startBox, int startChar, int endBox, int endChar)
{
	// 只在真的变化时重绘，避免拖拽时每次 mouseMove 都跑一次 paint
	if (startBox == selStartBox && startChar == selStartChar &&
		endBox == selEndBox && endChar == selEndChar) {
		return;
	}
	selStartBox = startBox;
	selStartChar = startChar;
	selEndBox = endBox;
	selEndChar = endChar;
	node->paint();
}
void ViewerText::syncSelectionToImg()
{
	auto img = ViewerImg::get();
	if (!img) return;
	// 纯 caret（没有真选中字符）不透传，避免 ViewerImg 侧“a==b 展开成一格”而
	// 出现“ViewerText 什么都没涂、ViewerImg 却有一个字符高亮”的不对称
	if (selStartBox == selEndBox && selStartChar == selEndChar) {
		img->setSelection(-1, -1, -1, -1);
		return;
	}
	img->setSelection(selStartBox, selStartChar, selEndBox, selEndChar);
}
void ViewerText::onSize(void* e)
{
	auto win = WindowMain::get();
	// 左边界跟随 ViewerImg 的右边界（分隔栏）
	float x = 0.f, w = 360.f;
	if (auto img = ViewerImg::get()) {
		x = img->getSplitX();
		w = std::max(0.f, win->w - x);
	}
	else {
		x = win->w - w;
	}
	float y{ 30.f * win->dpi }, h{ win->h - y - 22 * win->dpi };
	log(L"[ViewerText::onSize] win=({},{}) dpi={} -> node x={} y={} w={} h={}",
		win->w, win->h, win->dpi, x, y, w, h);
	node->setPosSize(x, y, w, h);
	log(L"[ViewerText::onSize] after setPosSize node absX={} absY={} absW={} absH={} visualOff=({},{}) visualSz=({},{})",
		node->absX, node->absY, node->absW, node->absH,
		node->visual.Offset().x, node->visual.Offset().y,
		node->visual.Size().x, node->visual.Size().y);
	setText(texts);
}

void ViewerText::onDown(void* e)
{
	if (node->isHoverScroller || !isHover) return;
	SetCapture(node->win->hwnd);
	isMouseDown = true;
	if (selStartBox >= 0) {
		selStartBox = selStartChar = selEndBox = selEndChar = -1;
		syncSelectionToImg();
		node->paint();
	}
	auto tuplePtr = static_cast<std::tuple<float, float, bool, Node*>*>(e);
	auto [x, y, isRight, nodePtr] = *tuplePtr;
	if (isRight) {
		return;
	}
	log(L"[ViewerText::onDown] wx={} wy={} scrollY={} nodeAbs=({},{}) visualOff=({},{}) visualSz=({},{})",
		x, y, node->scrollY,
		node->absX, node->absY,
		node->visual.Offset().x, node->visual.Offset().y,
		node->visual.Size().x, node->visual.Size().y);
	int boxIdx = -1, charIdx = -1;
	if (hitTest(x, y+node->scrollY, boxIdx, charIdx)) {
		selStartBox = boxIdx;
		selStartChar = charIdx;
		selEndBox = boxIdx;
		selEndChar = charIdx;
		log(L"[ViewerText::onDown] hit -> box={} char={}", boxIdx, charIdx);
		syncSelectionToImg();
	}
	else {
		log(L"[ViewerText::onDown] hitTest missed");
	}
}

void ViewerText::onUp(void* e)
{
	isMouseDown = false;
	ReleaseCapture();
}

void ViewerText::onMove(void* e)
{
	auto tuplePtr = static_cast<std::tuple<float,float>*>(e);
	auto [x, y] = *tuplePtr;
	auto padding = 12.f * node->win->dpi;
	auto size = node->visual.Size();
	if (x > node->absX + padding && x<node->absX + size.x - padding && y>node->absY && y < node->absY + size.y) {
		isHover = true;
	}
	else {
		isHover = false;
	}
	auto win = WindowMain::get();
	auto nodePos = node->visual.Offset();
	if (isMouseDown && selStartBox >= 0) {
		int boxIdx = -1, charIdx = -1;
		if (hitTest(x, y + node->scrollY, boxIdx, charIdx)) {
			if (boxIdx != selEndBox || charIdx != selEndChar) {
				selEndBox = boxIdx;
				selEndChar = charIdx;
				syncSelectionToImg();
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


	ComPtr<ID2D1SolidColorBrush> selBrush;
	ctx->CreateSolidColorBrush(ColorA(0x66AAFF88).getD2DColor(), selBrush.GetAddressOf());

	// 先绘制选中背景（放在文本下方，避免遮挡文字）
	if (selStartBox >= 0 && selEndBox >= 0) {
		int sBox = selStartBox, sChar = selStartChar;
		int eBox = selEndBox, eChar = selEndChar;
		if (sBox > eBox || (sBox == eBox && sChar > eChar)) {
			std::swap(sBox, eBox);
			std::swap(sChar, eChar);
		}
		for (int i = sBox; i <= eBox; i++) {
			if (i < 0 || i >= static_cast<int>(textLayouts.size())) continue;
			int len = static_cast<int>(textLens[i]);
			if (len <= 0) continue;
			int a = (i == sBox) ? sChar : 0;
			int b = (i == eBox) ? eChar : len;
			a = std::clamp(a, 0, len);
			b = std::clamp(b, 0, len);
			if (a == b) continue;
			UINT32 pos32 = static_cast<UINT32>(std::min(a, b));
			UINT32 cnt32 = static_cast<UINT32>(std::abs(b - a));
			// 先探测所需矩形数
			UINT32 actual = 0;
			DWRITE_HIT_TEST_METRICS probe;
			auto hr = textLayouts[i]->HitTestTextRange(pos32, cnt32,
				textPoss[i].x, textPoss[i].y, &probe, 1, &actual);
			if (actual == 0) continue;
			std::vector<DWRITE_HIT_TEST_METRICS> rects(actual);
			hr = textLayouts[i]->HitTestTextRange(pos32, cnt32,
				textPoss[i].x, textPoss[i].y, rects.data(), actual, &actual);
			if (FAILED(hr)) continue;
			for (auto& m : rects) {
				D2D1_RECT_F r = D2D1::RectF(m.left, m.top, m.left + m.width, m.top + m.height);
				ctx->FillRectangle(r, selBrush.Get());
			}
		}
	}

	for (size_t i = 0; i < textLayouts.size(); i++)
	{
		auto& layout = textLayouts[i];
		ctx->DrawTextLayout(textPoss[i], layout.Get(), brush.Get());
	}
}

void ViewerText::onCursor(void* e)
{
	if (isHover) {
		SetCursor(LoadCursor(nullptr, IDC_IBEAM));
		auto flag = static_cast<bool*>(e);
		*flag = true;
	}
}

void ViewerText::onKey(void* e)
{
	auto key = *static_cast<UINT*>(e);
	bool isCtrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	if (isCtrlDown && key == 'C') {
		if (selStartBox < 0 || selEndBox < 0) return;
		int sBox = selStartBox, sChar = selStartChar;
		int eBox = selEndBox, eChar = selEndChar;
		if (sBox > eBox || (sBox == eBox && sChar > eChar)) {
			std::swap(sBox, eBox);
			std::swap(sChar, eChar);
		}
		std::wstring str;
		for (int i = sBox; i <= eBox; i++) {
			if (i < 0 || i >= static_cast<int>(texts.size())) continue;
			int len = static_cast<int>(texts[i].size());
			if (len <= 0) continue;
			int a = (i == sBox) ? sChar : 0;
			int b = (i == eBox) ? eChar : len;
			a = std::clamp(a, 0, len);
			b = std::clamp(b, 0, len);
			if (a >= b) continue;
			if (!str.empty()) str += L"\r\n"; // 跨块之间加换行，粘贴到编辑器/Excel 更自然
			str.append(texts[i], a, b - a);
		}
		if (!str.empty()) Util::setTextToClipboard(str);
	}
}

bool ViewerText::hitTest(float wx, float wy, int& boxIdx, int& charIdx)
{
	if (textLayouts.empty()) return false;
	// 窗口坐标 -> 节点局部坐标
	auto nodeOff = node->visual.Offset();
	float lx = wx - nodeOff.x;
	float ly = wy - nodeOff.y;
	log(L"[hitTest] wx={} wy={} nodeOff=({},{}) -> lx={} ly={} layouts={}",
		wx, wy, nodeOff.x, nodeOff.y, lx, ly, textLayouts.size());
	// 按 y 找最近的文本块：先找严格包含 ly 的；若都不含，取 y 距离最近者
	int bestBox = -1;
	float bestDist = FLT_MAX;
	for (size_t i = 0; i < textLayouts.size(); i++) {
		float top = textPoss[i].y;
		float bottom = top + textHeights[i];
		float dist;
		if (ly >= top && ly <= bottom) dist = 0.f;
		else if (ly < top) dist = top - ly;
		else dist = ly - bottom;
		if (dist < bestDist) {
			bestDist = dist;
			bestBox = static_cast<int>(i);
		}
	}
	if (bestBox < 0) return false;
	log(L"[hitTest] bestBox={} textPos=({},{}) height={} textLen={}",
		bestBox, textPoss[bestBox].x, textPoss[bestBox].y, textHeights[bestBox], textLens[bestBox]);
	// 在选中的 layout 内做 HitTestPoint，得到字符索引
	// 越界的 ly 用块自身范围钳位，避免打到别的行
	float localX = lx - textPoss[bestBox].x;
	float localY = ly - textPoss[bestBox].y;
	localY = std::clamp(localY, 0.f, std::max(0.f, textHeights[bestBox] - 0.5f));
	BOOL isTrailing = FALSE, isInside = FALSE;
	DWRITE_HIT_TEST_METRICS metrics{};
	auto hr = textLayouts[bestBox]->HitTestPoint(localX, localY, &isTrailing, &isInside, &metrics);
	if (FAILED(hr)) return false;
	// 命中字符前沿 -> 光标位于 textPosition；命中后沿 -> 光标位于 textPosition + 1
	int pos = static_cast<int>(metrics.textPosition) + (isTrailing ? 1 : 0);
	pos = std::clamp(pos, 0, static_cast<int>(textLens[bestBox]));
	log(L"[hitTest] localX={} localY={} -> textPosition={} isTrailing={} pos={}",
		localX, localY, metrics.textPosition, (int)isTrailing, pos);
	boxIdx = bestBox;
	charIdx = pos;
	return true;
}
