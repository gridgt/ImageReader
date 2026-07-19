#include "pch.h"
#include "D2D.h"
#include "Node.h"
#include "WindowBase.h"

Node::Node(WindowBase* win, const std::string& id) :Event(), win{win}, id{id}
{
    visual = win->compositor.CreateSpriteVisual();
}

Node::~Node()
{
}

Node* Node::createChild(const std::string& id)
{
    auto insPtr = new Node(win, id);
    insPtr->parent = this;
    std::unique_ptr<Node> ptr(insPtr);
    visual.Children().InsertAtTop(insPtr->visual);
    children.push_back(std::move(ptr));
    return insPtr;
}

void Node::initSurface()
{
    auto d2d = D2D::get();
    surface = d2d->createDrawingSurface(win->compositor);
    Composition::CompositionSurfaceBrush brush = win->compositor.CreateSurfaceBrush(surface);
    visual.Brush(brush);
}

void Node::setPosSize(const float& x, const float& y, const float& w, const float& h)
{

    visual.Offset({ x,y,0.f });
    visual.Size({ w,h });
    if (surface) {
        surface.Resize({ static_cast<int>(w), static_cast<int>(h) }); //todo 尺寸没变就跳过
    }
}

bool Node::isPosIn(float x, float y)
{
    if (!visual.IsVisible()) return false;
    return x >= absX && x < absX + absW && y >= absY && y < absY + absH;
}

Node* Node::findLeafByPos(float x, float y)
{
    if (!visual.IsVisible() || !isPosIn(x, y)) return nullptr;
    // 从后往前遍历 children（后添加 = 视觉上层）
    for (int i = (int)children.size() - 1; i >= 0; --i) {
        auto* hit = children[i]->findLeafByPos(x, y);
        if (hit) return hit;
    }
    // 没有子节点命中，就是自己（叶子节点）
    return this;
}

void Node::hide()
{
    visual.IsVisible(false);
}

void Node::show()
{
    visual.IsVisible(true);
}

void Node::paint()
{
    auto s = surface.as<ABI::Windows::UI::Composition::ICompositionDrawingSurfaceInterop>();
    ComPtr<ID2D1DeviceContext> ctx;
    POINT offset{};   // 物理像素
    HRESULT hr = s->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), reinterpret_cast<void**>(ctx.GetAddressOf()), &offset);
    // 全程使用物理像素，不调 SetDpi
    auto trans = D2D1::Matrix3x2F::Translation(static_cast<float>(offset.x), static_cast<float>(offset.y));
    ctx->SetTransform(trans);
    ctx->Clear(0);
    auto arg = std::make_tuple<Node*, ID2D1DeviceContext*>(this, ctx.Get());
    emit("paint", &arg);
    s->EndDraw();
}

void Node::traverse(std::function<void(Node*)> visit)
{
    visit(this);
    for (auto& child : children) {
        child->traverse(visit);
    }
}

void Node::setBackgroundColor(const ColorA& color)
{
    visual.Brush(win->compositor.CreateColorBrush(color.getUIColor()));
}

bool Node::isVisible()
{
    return visual.IsVisible();
}

void Node::sizeChange()
{
    emit("sizeChange", this);
    if (parent) {
        // visual.Offset / visual.Size 是物理像素
        auto pos = visual.Offset();
        absX = parent->absX + pos.x;
        absY = parent->absY + pos.y;
        auto size = visual.Size();
        absW = size.x;
        absH = size.y;
    }
    else {
        // root：win->w/h 是物理像素
        absX = x;
        absY = y;
        absW = win->w;
        absH = win->h;
    }
    for (auto& child : children) {
        child->sizeChange();
    }
}

Node* Node::findLCA(Node* tar) {
    if (!tar) return nullptr;
    // 收集 a 的所有祖先（含自身）
    std::vector<Node*> ancestorsA;
    auto self = this;
    while (self->parent)
    {
        ancestorsA.push_back(self->parent);
        self = self->parent;
    }
    // 从 b 向上，找到第一个在 ancestorsA 中的节点
    for (auto* p = tar; p; p = p->parent) {
        for (auto* cand : ancestorsA) {
            if (cand == p) return cand;
        }
    }
    return nullptr; // 不应到达
}

std::vector<Node*> Node::pathUpTo(Node* stopAt)
{
    std::vector<Node*> path;
    auto node = this;
    while (node && node != stopAt) {
        path.push_back(node);
        node = node->parent;
    }
    return path; // [self, parent, grandparent, ..., stopAt.parent]
}
