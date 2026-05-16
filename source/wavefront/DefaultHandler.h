#pragma once

#include "Handler.h"

#include <vector>
#include <string>

class DefaultHandler : public Handler
{
public:
	bool Condition(const std::vector<std::string>& tokens) override;
	void Handle(const std::vector<std::string>& tokens) override;
	void Reset() override;
};
