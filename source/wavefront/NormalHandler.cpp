#include "pch.h"

#include "NormalHandler.h"

bool NormalHandler::Condition(const std::vector<std::string>& tokens)
{
	return (tokens[0].compare("vn") == 0);
}

void NormalHandler::Handle(const std::vector<std::string>& tokens)
{
	normals.push_back({ std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) });
}

void NormalHandler::Reset()
{
	normals.clear();
}