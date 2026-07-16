#pragma once
#include "pch.h"
#include "Event.h"
#include "ColorA.h"
class WindowBase;
class Node:public Event
{
	friend class WindowBase;
public:
	Node(WindowBase* win, const std::wstring& id = L"");
	virtual ~Node();
	Node* createChild(const std::wstring& id = L"");
	void initSurface();
	bool isPosIn(float x, float y);
	Node* findLeafByPos(float x, float y);
	void hide();
	void show();
	void traverse(std::function<void(Node*)> visit);
	void setBackgroundColor(const ColorA& color);
	bool isVisible();
	/// <summary>
	/// 查找目标节点与当前节点的共同祖先
	/// </summary>
	/// <param name="b"></param>
	/// <returns></returns>
	Node* findLCA(Node* tar);
	/// <summary>
	/// 从 node 向上到 stopAt（不含），收集祖先链表
	/// </summary>
	std::vector<Node*> pathUpTo(Node* stopAt);
public:
	WindowBase* win;
	std::wstring id;	
	/// <summary>
	/// 1. 将子元素的锚点设置为其自身的中心点 (0.5, 0.5)
	/// childVisual.AnchorPoint = new Vector2(0.5f, 0.5f);
	// 2. 将子元素的 Offset 设置为父容器的中心坐标
	/// childVisual.Offset = new Vector3(
	/// 	parentVisual.Size.X / 2,
	/// 	parentVisual.Size.Y / 2,
	/// 	0
	/// );
	// 3. 插入子元素
	/// parentVisual.Children.InsertAtTop(childVisual);
	/// </summary>
	Composition::SpriteVisual visual{ nullptr };
	std::vector<std::unique_ptr<Node>> children;
protected:
	std::pair<winrt::impl::com_ref<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>, ComPtr<ID2D1DeviceContext>> paintStart();
protected:
	float x, y, w, h;
	float absX, absY, absW, absH;
private:
	void sizeChange();
private:
	Node* parent;
	Composition::CompositionDrawingSurface surface{ nullptr };
};

