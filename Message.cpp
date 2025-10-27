#include "Message.h"
#include "WindowMain.h"

Message::Message(JsonObject&& param, ICoreWebView2* sender,WindowBase* win):
    param{ std::move(param) }, 
    sender{ sender },
    win{ win },
	mainThreadId{ GetCurrentThreadId() }
{
    initResult();
}

void Message::initResult()
{
    auto cbId = param.GetNamedString(L"_cbId");
    result.SetNamedValue(L"$$cbId", JsonValue::CreateStringValue(cbId));
    if (param.HasKey(L"_eventName"))
    {
        auto eName = param.GetNamedString(L"_eventName");
        result.SetNamedValue(L"$$eventName", JsonValue::CreateStringValue(eName));
    }
}

void Message::route()
{
    auto clsName = param.GetNamedString(L"_className");
    if (clsName == L"win") {
        win->exec(this);
    }
}

Message::~Message()
{
}

void Message::resolve()
{
    if(mainThreadId == GetCurrentThreadId()) {
        postMsgBack();
    } else {
        PostMessage(win->hwnd, MSG_BACK_ID, 0, (LPARAM)this);
	}
}

void Message::postMsgBack()
{
    //result.SetNamedValue(L"allen", JsonValue::CreateStringValue(L"allenallen"));
    auto resultStr = result.Stringify();    
    sender->PostWebMessageAsJson(resultStr.data());
    if (result.HasKey(L"$$eventName"))
    {
        if (result.HasKey(L"$$cbId")) {
			result.Remove(L"$$cbId");
        }
    }
    else {
        delete this;
    }
}




