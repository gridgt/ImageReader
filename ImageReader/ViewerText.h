#pragma once
#include "pch.h"
#include "Node.h"
class WindowBase;
class ViewerText
{
public:
	~ViewerText();
	static void init(WindowBase* win);
	static ViewerText* get();
	void setText(const std::vector<std::wstring>& texts);
private:
	ViewerText(WindowBase* win);
	void onSize(void* e);
	void onDown(void* e);
	void onUp(void* e);
	void onMove(void* e);
	void onPaint(void* e);
	void onCursor(void* e);
	/// <summary>
	/// 将窗口坐标转换为图像坐标下最近的 box + 分隔线索引
	/// </summary>
	bool hitTest(float wx, float wy, int& boxIdx, int& charIdx);
private:
	D2D1_POINT_2F pos;
	bool isHover{ false }, isMouseDown{false};
	Node* node;
	std::vector<ComPtr<IDWriteTextLayout>> textLayouts;
	std::vector<D2D1_POINT_2F> textPoss;
	// 文本选区，Char 索引是 charLines[box] 中的“分隔线”索引，
	// 字符 j 被选中当且仅当 min(startChar,endChar) <= j < max(startChar,endChar)
	int selStartBox{ -1 }, selStartChar{ -1 };
	int selEndBox{ -1 }, selEndChar{ -1 };
};

