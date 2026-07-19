#pragma once
#include "pch.h"
#include "Node.h"
class WindowBase;
class Loader
{
public:
	~Loader();
	static void init(WindowBase* win);
	static Loader* get();
private:
	Loader(WindowBase* win);
	void onSize(void* e);
	void onDown(void* e);
	void onPaint(void* e);
	std::wstring getFilePath();
	std::vector<unsigned char> getFileData(const std::wstring& filePath);
private:
	ComPtr<IDWriteTextLayout> text;
	D2D1_POINT_2F textPos;
	bool isHover{false};
	Node* node;
};

