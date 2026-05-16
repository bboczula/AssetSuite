#include "pch.h"

#include "TextureHandler.h"

bool TextureHandler::Condition(const std::vector<std::string>& tokens)
{
	return (tokens[0].compare("vt") == 0);
}

void TextureHandler::Handle(const std::vector<std::string>& tokens)
{
	textureCoord.push_back({ std::stof(tokens[1]), std::stof(tokens[2]) });
}

void TextureHandler::Reset()
{
	textureCoord.clear();
}

MiniMath::Point2D TextureHandler::GetTextureCoordAt(unsigned int index)
{
	return textureCoord[index];
}
