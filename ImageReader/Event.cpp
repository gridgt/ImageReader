#include "pch.h"
#include "Event.h"

size_t Event::onMouseMove(std::function<void(const MouseEventArg&)> callback)
{
	mouseMoveCBId += 1;
	mouseMoveCBs.insert({ mouseMoveCBId,callback });
	return mouseMoveCBId;
}

size_t Event::onMouseDown(std::function<void(const MouseEventArg&)> callback)
{
	mouseDownCBId += 1;
	mouseDownCBs.insert({ mouseDownCBId,callback });
	return mouseDownCBId;
}

size_t Event::onMouseUp(std::function<void(const MouseEventArg&)> callback)
{
	mouseDownCBId += 1;
	mouseUpCBs.insert({ mouseUpCBId,callback });
	return mouseDownCBId;
}

size_t Event::onMouseEnter(std::function<void(const MouseEventArg&)> callback)
{
	mouseEnterCBId += 1;
	mouseEnterCBs.insert({ mouseEnterCBId,callback });
	return mouseEnterCBId;
}

size_t Event::onMouseLeave(std::function<void(const EventArg&)> callback)
{
	mouseLeaveCBId += 1;
	mouseLeaveCBs.insert({ mouseLeaveCBId,callback });
	return mouseLeaveCBId;
}

size_t Event::onShown(std::function<void(const EventArg&)> callback)
{
	shownCBId += 1;
	shownCBs.insert({ mouseLeaveCBId,callback });
	return mouseLeaveCBId;
}

size_t Event::onSizeChange(std::function<void(const EventArg&)> callback)
{
	sizeChangeCBId += 1;
	sizeChangeCBs.insert({ sizeChangeCBId,callback });
	return sizeChangeCBId;
}

void Event::offMouseEnter(const size_t& callbackId)
{
	mouseEnterCBs.erase(callbackId);
}
void Event::offMouseLeave(const size_t& callbackId)
{
	mouseLeaveCBs.erase(callbackId);
}
void Event::offMouseMove(const size_t& callbackId)
{
	mouseMoveCBs.erase(callbackId);
}
void Event::offMouseDown(const size_t& callbackId)
{
	mouseDownCBs.erase(callbackId);
}
void Event::offMouseUp(const size_t& callbackId)
{
	mouseUpCBs.erase(callbackId);
}
void Event::offShown(const size_t& callbackId)
{
	shownCBs.erase(callbackId);
}

void Event::offSizeChange(const size_t& callbackId)
{
	sizeChangeCBs.erase(callbackId);
}

void Event::mouseEnter(const MouseEventArg& arg)
{
	for (const auto& pair : mouseEnterCBs) {
		pair.second(arg);
	}
}

void Event::mouseLeave(const EventArg& arg)
{
	for (const auto& pair : mouseLeaveCBs) {
		pair.second(arg);
	}
}

void Event::mouseMove(const MouseEventArg& arg)
{
	for (const auto& pair : mouseMoveCBs) {
		pair.second(arg);
	}
}

void Event::mouseDown(const MouseEventArg& arg)
{
	for (const auto& pair : mouseDownCBs) {
		pair.second(arg);
	}
}

void Event::mouseUp(const MouseEventArg& arg)
{
	for (const auto& pair : mouseUpCBs) {
		pair.second(arg);
	}
}

void Event::shown(const EventArg& arg)
{
	for (const auto& pair : shownCBs) {
		pair.second(arg);
	}
}

void Event::sizeChange(const EventArg& arg)
{
	for (const auto& pair : sizeChangeCBs) {
		pair.second(arg);
	}
}
