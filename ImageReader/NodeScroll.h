#pragma once
#include "pch.h"
#include "Node.h"
#include "ColorA.h"
class WindowBase;
class NodeScroll:public Node
{
	friend class WindowBase;
public:
	NodeScroll(WindowBase* win, const std::string& id = "");
	virtual ~NodeScroll();
	void setContentPosSize(const float& x, const float& y, const float& w, const float& h);
	void initContentSurface();
	void onPaint(void* e);
	void onWheel(void* e);
	void onDown(void* e);
	void onUp(void* e);
	void onMove(void* e);
	void onCursor(void* e);
	bool hasScroller();
	void setScroll(float y);

public:
	Composition::SpriteVisual visualContent{ nullptr };
	Composition::SpriteVisual visualScroller{ nullptr };
	Composition::CompositionDrawingSurface surfaceScroller{ nullptr };
	float scrollY, dragStartMouseY, dragStartScrollY;
	bool isHoverScroller, scrollerDragging;
private:
	void paintScrollbar();
private:
};

