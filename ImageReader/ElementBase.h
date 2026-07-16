#pragma once
#include "pch.h"

class WindowCalendar;
class ElementBase
{
public:
	ElementBase(WindowCalendar* win,const std::wstring& id);
	virtual ~ElementBase();
	virtual void onSizeChange() {};
	virtual bool setCursor() { return false; };
	virtual void onMouseMove(const int& x, const int& y) {};
	virtual void onMouseDown(const int& x, const int& y, bool isRight) {};
	virtual void onMouseUp(const int& x, const int& y) {};
	virtual void onMouseDrag(const int& x, const int& y, const UINT_PTR& modifiers) {};
	virtual void onMouseWheel(const int& x, const int& y, const short& delta) {};
	virtual void onData(const JsonObject& jsonObj) {};
	void hide();
	void show();
	bool isVisible();
public:
	std::wstring id;
	Composition::SpriteVisual visual{ nullptr };
protected:
	std::pair<winrt::impl::com_ref<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>, ComPtr<ID2D1DeviceContext>> paintStart(Composition::CompositionDrawingSurface& surface);
protected:
	WindowCalendar* win;
};

