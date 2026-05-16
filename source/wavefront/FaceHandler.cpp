#include "pch.h"

#include "FaceHandler.h"

bool FaceHandler::Condition(const std::vector<std::string>& tokens)
{
	return (tokens[0].compare("f") == 0);
}

void FaceHandler::Handle(const std::vector<std::string>& tokens)
{
	bool isTiangulationNeeded = false;
	if (tokens.size() == 5)
	{
		isTiangulationNeeded = true;
	}

	Face face;

	for (int i = 1; i < tokens.size(); i++)
	{
		std::vector<std::string> innerTokens;
		char* first;
		char* next = NULL;
		first = strtok_s((char*)tokens[i].c_str(), "/", &next);

		do
		{
			innerTokens.push_back(first);
			if (next[0] == '/')
			{
				// This means that we have empty element
				innerTokens.push_back(std::string());
			}
		} while ((first = strtok_s(NULL, "/", &next)) != NULL);

		if (innerTokens.size() == 1)
		{
			face.vertexIndex.push_back(std::stoi(innerTokens[0]) - 1);
		}
		else if (innerTokens.size() == 2)
		{
			face.vertexIndex.push_back(std::stoi(innerTokens[0]) - 1);
			face.textureIndex.push_back(std::stoi(innerTokens[1]) - 1);
		}
		else if (innerTokens.size() == 3)
		{
			face.vertexIndex.push_back(std::stoi(innerTokens[0]) - 1);
			if (!innerTokens[1].empty())
			{
				face.textureIndex.push_back(std::stoi(innerTokens[1]) - 1);
			}
			face.normalIndex.push_back(std::stoi(innerTokens[2]) - 1);
		}
	}

	if (!isTiangulationNeeded)
	{
		faces.push_back(face);
	}
	else
	{
		auto tempIndex = face.vertexIndex.back();
		face.vertexIndex.pop_back();

		Face additional;
		additional.vertexIndex.push_back(face.vertexIndex[0]);
		additional.vertexIndex.push_back(face.vertexIndex[2]);
		additional.vertexIndex.push_back(tempIndex);
		if (face.textureIndex.size())
		{
			auto tempTexCoordIndex = face.textureIndex.back();
			face.textureIndex.pop_back();
			additional.textureIndex.push_back(face.textureIndex[0]);
			additional.textureIndex.push_back(face.textureIndex[2]);
			additional.textureIndex.push_back(tempTexCoordIndex);
		}
		if (face.normalIndex.size())
		{
			auto tempNormalIndex = face.normalIndex.back();
			face.normalIndex.pop_back();
			additional.normalIndex.push_back(face.normalIndex[0]);
			additional.normalIndex.push_back(face.normalIndex[2]);
			additional.normalIndex.push_back(tempNormalIndex);
		}

		faces.push_back(face);
		faces.push_back(additional);
	}
}

void FaceHandler::Reset()
{
	faces.clear();
}