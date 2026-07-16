#include "pch.h"
#include "TitleBar.h"
#include "WindowBase.h"
#include "Node.h"

static std::unique_ptr<TitleBar> ins;

TitleBar::TitleBar(WindowBase* win)
{
	btn0 = win->root->createChild();
	btn1 = win->root->createChild();
	btn2 = win->root->createChild();

	//btn0->onSizeChange()
}

TitleBar::~TitleBar()
{
}

void TitleBar::init(WindowBase* win)
{
	ins.reset(new TitleBar(win));

}
