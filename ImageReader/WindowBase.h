#pragma once
#include "pch.h"
#include "Event.h"

class Node;
class WindowBase: public Event
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
	int x{ 0 }, y{ 0 };      // 屏幕坐标：物理像素
	float w{ 0 }, h{ 0 };    // 客户区大小：物理像素
	float dpi{ 1.0 };
	HWND hwnd{ nullptr };
	std::wstring title;
	Composition::Compositor compositor;
	std::unique_ptr<Node> root;
protected:
	virtual void onCreated() {};
	virtual void onShown() {};
	virtual void onHidden();
	virtual LRESULT onHitTest(const float& x, const float& y) { return HTCLIENT; };
	virtual void onMouseWheel(const float& x, const float& y, const short& delta) {};
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
	virtual void onMinMaxInfo(MINMAXINFO* mmi) {};
private:
	std::wstring& getWinClsName();
	static LRESULT CALLBACK winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void mouseMove(const float& x, const float& y);
	void mouseLeave();
	void mouseDown(const float& x, const float& y, bool isRight);
	void mouseUp(const float& x, const float& y, bool isRight);
	void paint();
	void dpiChange(WPARAM wParam, LPARAM lParam);
	void sizeChange(WPARAM wParam, LPARAM lParam);
	void positionChange(const int& x, const int& y);
private:
	Composition::Desktop::DesktopWindowTarget winTarget{ nullptr };	
	Node* nodeHover{nullptr};
	bool isMouseIn{ false }, wasMaximized{false};
};

