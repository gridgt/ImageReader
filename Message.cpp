#include "Message.h"
#include "WindowMain.h"

Message::Message(JsonObject&& param, WindowMain* win):
    param{ std::move(param) }, 
    win{ win },
    uiDQ{ winrt::Windows::System::DispatcherQueue::GetForCurrentThread() }
{
    initResult();
}

void Message::initResult()
{
    auto cbId = param.GetNamedString(L"$cbId");
    result.SetNamedValue(L"$cbId", JsonValue::CreateStringValue(cbId));
    if (param.HasKey(L"$eventName"))
    {
        auto eName = param.GetNamedString(L"$eventName");
        result.SetNamedValue(L"$eventName", JsonValue::CreateStringValue(eName));
    }
}

void Message::route()
{
    auto clsName = param.GetNamedString(L"$className");
    if (clsName == L"win") {
        win->exec(this);
    }
}

Message::~Message()
{
}

void Message::resolve()
{
    uiDQ.TryEnqueue([this]()
        {
            auto resultStr = result.Stringify();
            win->webview->PostWebMessageAsJson(resultStr.data());
            if (result.HasKey(L"$eventName"))
            {
                if (result.HasKey(L"$cbId")) {
                    result.Remove(L"$cbId");
                }
            }
            else {
                delete this;
            }
        });
}




