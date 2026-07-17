#include "pch.h"
#include "Event.h"

Event::Event()
{
}

Event::~Event()
{
}


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

size_t Event::on(const std::string& eventName, std::function<void(void*)> cb)
{
	// 每个 Event 实例自己维护计数器，避免跨实例冲突
	auto id = ++nextId;
	events[eventName].push_back({ id, std::move(cb) });
	return id;
}

void Event::off(const std::string& eventName, const size_t& id)
{
	auto it = events.find(eventName);
	if (it == events.end()) return;
	auto& vec = it->second;
	// 按 id 摘除；用 erase-remove_if 一次遍历完成
	vec.erase(
		std::remove_if(vec.begin(), vec.end(),
			[id](const Slot& s) { return s.id == id; }),
		vec.end()
	);
	// 该事件没剩下的回调，把 key 也删了，避免 map 累积空 vector
	if (vec.empty()) events.erase(it);
}

void Event::emit(const std::string& eventName, void* arg)
{
	auto it = events.find(eventName);
	if (it == events.end()) return;
	// 禁止在回调内 on/off 自身
	auto& slots = it->second;
	for (const auto& s : slots) {
		s.cb(arg);
	}
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

void Event::mouseEnter(MouseEventArg& arg)
{
	arg.target = this;
	for (const auto& pair : mouseEnterCBs) {
		pair.second(arg);
	}
}

void Event::mouseLeave(EventArg& arg)
{
	arg.target = this;
	for (const auto& pair : mouseLeaveCBs) {
		pair.second(arg);
	}
}

void Event::mouseMove(MouseEventArg& arg)
{
	arg.target = this;
	for (const auto& pair : mouseMoveCBs) {
		pair.second(arg);
	}
}

void Event::mouseDown(MouseEventArg& arg)
{
	arg.target = this;
	for (const auto& pair : mouseDownCBs) {
		pair.second(arg);
	}
}

void Event::mouseUp(MouseEventArg& arg)
{
	arg.target = this;
	for (const auto& pair : mouseUpCBs) {
		pair.second(arg);
	}
}

void Event::shown(EventArg& arg)
{
	arg.target = this;
	for (const auto& pair : shownCBs) {
		pair.second(arg);
	}
}

void Event::sizeChange(EventArg& arg)
{
	arg.target = this;
	for (const auto& pair : sizeChangeCBs) {
		pair.second(arg);
	}
}
