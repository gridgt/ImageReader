#pragma once
#include "pch.h"

class Node;
class WindowBase
{
public:
	WindowBase();
	virtual ~WindowBase();
	void enableShadow();
	void show();
	void hide();
	void refresh();
	void close();
	void minimize();
	void move(const int& x, const int& y);
	void resize(const int& w, const int& h);
	void createNativeWindow(const DWORD& exStyle = NULL, const DWORD& style = NULL);
	void setTimer(const UINT& elapse, const UINT& id);
	void killTimer(const UINT& id);
	void setTitle(const std::wstring& title);
	std::wstring getTitle();
	std::tuple<int, int> getPosition();
	std::tuple<float, float> getSize();
	void setSize(const float& w, const float& h);
	void setPosition(const int& x, const int& y);
	HWND getHandle();
	float getScaleFactor();
	void setPosScreenCenter();
public:
	int x, y, w, h;
	float dpi{ 1.0 };
	HWND hwnd{ nullptr };
	std::wstring title;
	Composition::Compositor compositor;
	std::unique_ptr<Node> root;
protected:
	virtual void onCreated() {};
	virtual void onShown() {};
	virtual void onHidden();
	virtual LRESULT onHitTest(const int& x, const int& y) { return HTCLIENT; };
	virtual void onMouseWheel(const int& x, const int& y, const short& delta) {};
	virtual void onKeyDown(const UINT& key) {};
	virtual void onKeyUp() {};
	virtual void onChar(const UINT& ch) {};
	virtual void onTimer(const UINT& timerId) {};
	virtual void onIme() {};
	virtual void onBlur() {};
	virtual void onDestroy() {};
	virtual void onDpiChanged() {};
	virtual void onPositionChange() {};
	virtual BOOL setCursor();
private:
	std::wstring& getWinClsName();
	static LRESULT CALLBACK winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void mouseMove(const int& x, const int& y);
	void mouseLeave();
	void mouseDown(const int& x, const int& y, bool isRight);
	void mouseUp(const int& x, const int& y, bool isRight);
	void paint();
	void dpiChange(WPARAM wParam, LPARAM lParam);
	void sizeChange(const int& w, const int& h);
	void positionChange(const int& x, const int& y);
private:
	Composition::Desktop::DesktopWindowTarget winTarget{ nullptr };	
	Node* nodeHover{nullptr};
	bool isMouseIn{ false };
};

