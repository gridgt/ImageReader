#pragma once
#include "pch.h"
#include "Node.h"
class WindowBase;
class StatusBar
{
public:
	~StatusBar();
	static void init(WindowBase* win);
	static StatusBar* get();
	/// <summary>
	/// 显示“正在识别图像中的文字...”
	/// </summary>
	void showRecognizing();
	/// <summary>
	/// 显示“图像识别完成，耗时：xxx毫秒”
	/// </summary>
	void showRecognizeDone(long long elapsedMs);
	/// <summary>
	/// 显示任意文本，通用入口。
	/// </summary>
	void setText(const std::wstring& text);
private:
	StatusBar(WindowBase* win);
	void onSize(void* e);
	void onPaint(void* e);
	void buildLayout(); // 用当前 text + 当前尺寸/dpi 重建 IDWriteTextLayout
private:
	Node* node;
	std::wstring text;
	ComPtr<IDWriteTextLayout> layout;
	D2D1_POINT_2F textPos{ 0.f, 0.f };
};
