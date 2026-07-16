#pragma once
class Event;
class EventArg
{
public:
	EventArg();
	virtual ~EventArg();
public:
	bool stopPopup{ false };
	Event* target;
private:

};

