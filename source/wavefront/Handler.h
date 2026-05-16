#pragma once

#include <vector>
#include <string>

class Handler
{
public:
	Handler() : next(nullptr)
	{
	}
	Handler* SetNext(Handler* handler)
	{
		return next = handler;
	}
	virtual bool Condition(const std::vector<std::string>& tokens) = 0;
	virtual void Handle(const std::vector<std::string>& tokens) = 0;
	virtual void Reset() = 0;
	Handler* next;
};