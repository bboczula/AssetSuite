#pragma once

#include "Handler.h"
#include "FaceHandler.h"

#include <vector>
#include <string>

// You can keep pointers to the std::vectors of the groups, but then you would depend on the FaceHandler
// You can also store offset to the face vector, but how? You still have to relate to the FaceHandler
class GroupHandler : public Handler
{
public:
	GroupHandler(FaceHandler* faceHandler);
	bool Condition(const std::vector<std::string>& tokens);
	void Handle(const std::vector<std::string>& tokens) override;
	void Reset() override;
	unsigned int GetGroupOffset(const std::string& key);
	unsigned int GetGroupSize(const std::string& key);
	bool HasGroup(const std::string& key);
	FaceHandler* faceHandler;
	std::vector<std::pair<std::string, unsigned int>> groupOffset;
};