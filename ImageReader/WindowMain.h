#pragma once
#include <include/Ling.h>
class TitleBar;
class StatusBar;
class TextBox;
class ImgViewer;
class WindowMain : public Ling::WinBase
{
public:
	~WindowMain();
	static void init();
	static WindowMain* get();
public:
	ImgViewer* imgViewer;
	TextBox* textBox;
private:
	WindowMain();
	void onCreated() override;
	LRESULT onHitTest(const POINT pos) override;
	void onSetCursor(bool* flag);
	void onDown(POINT pt, bool isRight);
	void onMove(POINT pt);
	void onUp(POINT pt);
private:
	TitleBar* titleBar;
	StatusBar* statusBar;
	Ling::Node* splitter;
	bool isDragging{ false };
};

