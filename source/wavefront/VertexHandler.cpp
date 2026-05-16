#include "pch.h"

#include "VertexHandler.h"

bool VertexHandler::Condition(const std::vector<std::string>& tokens)
{
	return (tokens[0].compare("v") == 0);
}

void VertexHandler::Handle(const std::vector<std::string>& tokens)
{
	vertices.push_back({ std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) });
}

void VertexHandler::Reset()
{
	vertices.clear();
}

MiniMath::Point3D VertexHandler::GetVertexAt(unsigned int index)
{
	return vertices[index];
}