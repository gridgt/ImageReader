#pragma once
#include "pch.h"
#include "EventArg.h"
#include "MouseEventArg.h"
class Event
{
public:
	Event();
	virtual ~Event();
	size_t on(const std::string& eventName, std::function<void(void*)> cb);
	void off(const std::string& eventName, const size_t& id);
	void emit(const std::string& eventName, void* arg);





	size_t onMouseEnter(std::function<void(const MouseEventArg&)> callback);
	size_t onMouseLeave(std::function<void(const EventArg&)> callback);
	size_t onMouseMove(std::function<void(const MouseEventArg&)> callback);
	size_t onMouseDown(std::function<void(const MouseEventArg&)> callback);
	size_t onMouseUp(std::function<void(const MouseEventArg&)> callback);
	size_t onShown(std::function<void(const EventArg&)> callback);
	size_t onSizeChange(std::function<void(const EventArg&)> callback);

	void offMouseEnter(const size_t& callbackId);
	void offMouseLeave(const size_t& callbackId);
	void offMouseMove(const size_t& callbackId);
	void offMouseDown(const size_t& callbackId);
	void offMouseUp(const size_t& callbackId);
	void offShown(const size_t& callbackId);	
	void offSizeChange(const size_t& callbackId);

	void mouseEnter(MouseEventArg& arg);
	void mouseLeave(EventArg& arg);
	void mouseMove(MouseEventArg& arg);
	void mouseDown(MouseEventArg& arg);
	void mouseUp(MouseEventArg& arg);
	void shown(EventArg& arg);
	void sizeChange(EventArg& arg);
public:
protected:

private:
	size_t mouseMoveCBId{ 0 };
	std::unordered_map<size_t, std::function<void(const MouseEventArg&)>> mouseMoveCBs;
	size_t mouseDownCBId{ 0 };
	std::unordered_map<size_t, std::function<void(const MouseEventArg&)>> mouseDownCBs;
	size_t mouseUpCBId{ 0 };
	std::unordered_map<size_t, std::function<void(const MouseEventArg&)>> mouseUpCBs;
	size_t mouseEnterCBId{ 0 };
	std::unordered_map<size_t, std::function<void(const MouseEventArg&)>> mouseEnterCBs;
	size_t mouseLeaveCBId{ 0 };
	std::unordered_map<size_t, std::function<void(const EventArg&)>> mouseLeaveCBs;
	size_t shownCBId{ 0 };
	std::unordered_map<size_t, std::function<void(const EventArg&)>> shownCBs;
	size_t sizeChangeCBId{ 0 };
	std::unordered_map<size_t, std::function<void(const EventArg&)>> sizeChangeCBs;

	struct Slot
	{
		size_t id;
		std::function<void(void*)> cb;
	};

	// 一个事件名可挂多个回调，按注册顺序派发
	std::unordered_map<std::string, std::vector<Slot>> events;
	size_t nextId{ 0 };   // 每实例独立的 id 计数器
};

