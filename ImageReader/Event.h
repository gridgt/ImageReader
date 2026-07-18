#pragma once
#include "pch.h"
class Event
{
public:
	Event();
	virtual ~Event();
	size_t on(const std::string& eventName, std::function<void(void*)> cb);
	void off(const std::string& eventName, const size_t& id);
	void emit(const std::string& eventName, void* arg);
public:
private:
	struct Slot
	{
		size_t id;
		std::function<void(void*)> cb;
	};
	std::unordered_map<std::string, std::vector<Slot>> events;
	size_t nextId{ 0 };
};

