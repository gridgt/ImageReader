#pragma once
#include "pch.h"
#include "NodeScroll.h"
class WindowBase;
class ViewerText
{
public:
	~ViewerText();
	static void init(WindowBase* win);
	static ViewerText* get();
	void setText(const std::vector<std::wstring>& texts);
	/// <summary>
	/// 外部（如 ViewerImg）同步选区。传入负值等同于清空选区。
	/// startBox/endBox 与 setText 时的下标一致；startChar/endChar 是各自 layout 内的字符位置。
	/// </summary>
	void setSelection(int startBox, int startChar, int endBox, int endChar);
private:
	ViewerText(WindowBase* win);
	void onSize(void* e);
	void onDown(void* e);
	void onUp(void* e);
	void onMove(void* e);
	void onPaint(void* e);
	void onCursor(void* e);
	void onKey(void* e);
	/// <summary>
	/// 将窗口坐标转换为图像坐标下最近的 box + 分隔线索引
	/// </summary>
	bool hitTest(float wx, float wy, int& boxIdx, int& charIdx);
	/// <summary>
	/// 把当前选区同步到 ViewerImg（选区无效时清空 ViewerImg 选区）
	/// </summary>
	void syncSelectionToImg();
private:
	D2D1_POINT_2F pos;
	bool isHover{ false }, isMouseDown{false};
	NodeScroll* node;
	std::vector<ComPtr<IDWriteTextLayout>> textLayouts;
	std::vector<D2D1_POINT_2F> textPoss;
	std::vector<UINT32> textLens;    // 每个 layout 的字符数（供 HitTestTextRange 做区间截断）
	std::vector<float> textHeights;  // 每个 layout 的高度（供 hitTest 按 y 找块）
	std::vector<std::wstring> texts; // 原始文本，供复制到剪贴板
	// 文本选区：selStartChar/selEndChar 是该块内的“字符位置”（即光标位置），
	// 字符 j 被选中当且仅当 min(startChar,endChar) <= j < max(startChar,endChar)
	int selStartBox{ -1 }, selStartChar{ -1 };
	int selEndBox{ -1 }, selEndChar{ -1 };
};

