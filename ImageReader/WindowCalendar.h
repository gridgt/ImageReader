#pragma once
#include "WindowBase.h"
class TitleBar;
class TitleBtn;
class ElementBase;
class WindowCalendar : public WindowBase
{
public:
	~WindowCalendar();
	static void init();
	static WindowCalendar* get();
	void switchEmbed();
	void hideSchedule();
	void showSchedule();
	void onData(const JsonObject& jsonObj);
	ElementBase* getChildById(const std::wstring& id);
public:
	bool isEmbeded{ false };
	std::wstring clickToCompleteTodo, clickToRestartTodo;
private:
	WindowCalendar();
	void onSizeChange() override;
	void onCreated() override;
	void onMouseDown(const int& x, const int& y, bool isRight) override;
	void onMouseMove(const int& x, const int& y) override;
	void onMouseUp(const int& x, const int& y) override;
	void onMouseDrag(const int& x, const int& y, const UINT_PTR& modifiers) override;
	void onMouseWheel(const int& x, const int& y, const short& delta) override;
	void onMouseLeave() override;
	void onTimer(const UINT& timerId) override;
	LRESULT onHitTest(const int& x, const int& y) override;
	BOOL setCursor() override;
	static LRESULT CALLBACK processMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void embed();
	void unembed();
private:
	Composition::CompositionDrawingSurface surface{ nullptr };
	std::vector<std::unique_ptr<ElementBase>> elements;
	POINT pressPos, embedPosition;
	bool hasData{ false };
	int logicW{ 372 }, logicH{ 680 };
};

