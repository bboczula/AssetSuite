#include "pch.h"

#include "GroupHandler.h"

GroupHandler::GroupHandler(FaceHandler* faceHandler) : faceHandler(faceHandler)
{
}

bool GroupHandler::Condition(const std::vector<std::string>& tokens)
{
	const char id[] = "g";
	return (tokens[0].compare(id) == 0);
}

void GroupHandler::Handle(const std::vector<std::string>& tokens)
{
	groupOffset.push_back(std::make_pair(tokens[1], static_cast<unsigned int>(faceHandler->faces.size())));
}

void GroupHandler::Reset()
{
	groupOffset.clear();
}

unsigned int GroupHandler::GetGroupOffset(const std::string& key)
{
	for (auto i = 0; i < groupOffset.size(); i++)
	{
		if (groupOffset[i].first.compare(key) == 0)
		{
			return groupOffset[i].second;
		}
	}
	return 0;
}

unsigned int GroupHandler::GetGroupSize(const std::string& key)
{
	for (int i = 0; i < groupOffset.size(); i++)
	{
		if (groupOffset[i].first.compare(key) == 0)
		{
			auto currentGroupOffset = groupOffset[i].second;
			if (i == groupOffset.size() - 1)
			{
				return static_cast<unsigned int>(faceHandler->faces.size()) - currentGroupOffset;
			}
			else
			{
				return groupOffset[i + 1].second - currentGroupOffset;
			}
		}
	}
	return 0;
}

bool GroupHandler::HasGroup(const std::string& key)
{
	for (int i = 0; i < groupOffset.size(); i++)
	{
		if (groupOffset[i].first.compare(key) == 0)
		{
			return true;
		}
	}
	
	return false;
}
