#pragma once

#include <vector>
#include <string>

#include "Handler.h"
#include "MiniMath.h"

class NormalHandler : public Handler
{
public:
	bool Condition(const std::vector<std::string>& tokens);
	void Handle(const std::vector<std::string>& tokens) override;
	void Reset() override;
	std::vector<MiniMath::Vector3D> normals;
	std::vector<MiniMath::Vector3D> tangents;
};