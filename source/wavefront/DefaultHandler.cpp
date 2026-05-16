#include "pch.h"

#include "DefaultHandler.h"

bool DefaultHandler::Condition(const std::vector<std::string>& tokens)
{
	return true;
}

void DefaultHandler::Handle(const std::vector<std::string>& tokens)
{
}

void DefaultHandler::Reset()
{
}