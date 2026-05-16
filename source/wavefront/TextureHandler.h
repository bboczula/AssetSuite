#pragma once

#pragma once

#include "Handler.h"
#include "MiniMath.h"

#include <vector>
#include <string>

class TextureHandler : public Handler
{
public:
	bool Condition(const std::vector<std::string>& tokens) override;
	void Handle(const std::vector<std::string>& tokens) override;
	void Reset() override;
	MiniMath::Point2D GetTextureCoordAt(unsigned int index);
	std::vector<MiniMath::Point2D> textureCoord;
};
