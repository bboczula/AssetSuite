#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "Handler.h"
#include "MiniMath.h"

class VertexHandler : public Handler
{
public:
	bool Condition(const std::vector<std::string>& tokens) override;
	void Handle(const std::vector<std::string>& tokens) override;
	void Reset() override;
	MiniMath::Point3D GetVertexAt(unsigned int index);
	std::vector<MiniMath::Point3D> vertices;
};