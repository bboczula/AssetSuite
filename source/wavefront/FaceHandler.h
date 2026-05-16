#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "Handler.h"

class Face
{
public:
	bool IsConvex();
	bool IsTriangle();
	bool IsQuad();
	friend std::ostream& operator<<(std::ostream& os, const Face& face)
	{
		os << face.vertexIndex[0] + 1 << "//" << face.normalIndex[0] + 1
			<< " " << face.vertexIndex[1] + 1 << "//" << face.normalIndex[1] + 1
			<< " " << face.vertexIndex[2] + 1 << "//" << face.normalIndex[2] + 1;
		return os;
	}
	std::vector<unsigned int> vertexIndex;
	std::vector<unsigned int> textureIndex;
	std::vector<unsigned int> normalIndex;
	std::vector<unsigned int> tangentIndex;
};

class FaceHandler : public Handler
{
public:
	bool Condition(const std::vector<std::string>& tokens);
	void Handle(const std::vector<std::string>& tokens) override;
	void Reset() override;
	std::vector<Face> faces;
};