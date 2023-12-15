#include "pch.h"

#include "ModelLoader.h"

AssetSuite::ModelLoader::ModelLoader()
	: vertexHandler(nullptr), normalHandler(nullptr), faceHandler(nullptr), groupHandler(nullptr), defaultHandler(nullptr), textureHandler(nullptr), currentFile("")
{
	vertexHandler = new VertexHandler;
	normalHandler = new NormalHandler;
	faceHandler = new FaceHandler;
	groupHandler = new GroupHandler(faceHandler);
	textureHandler = new TextureHandler;
	defaultHandler = new DefaultHandler;

	// Define chain of responsibility
	vertexHandler->SetNext(normalHandler)->SetNext(textureHandler)->SetNext(faceHandler)->SetNext(groupHandler)->SetNext(defaultHandler);
}

AssetSuite::ModelLoader::~ModelLoader()
{
	if (vertexHandler)
	{
		delete vertexHandler;
		vertexHandler = nullptr;
	}

	if (normalHandler)
	{
		delete normalHandler;
		normalHandler = nullptr;
	}

	if (faceHandler)
	{
		delete faceHandler;
		faceHandler = nullptr;
	}

	if (groupHandler)
	{
		delete groupHandler;
		groupHandler = nullptr;
	}

	if (textureHandler)
	{
		delete textureHandler;
		textureHandler = nullptr;
	}

	if (defaultHandler)
	{
		delete defaultHandler;
		defaultHandler = nullptr;
	}
}

void AssetSuite::ModelLoader::LoadFromFile(std::string filePath)
{
	if (currentFile.compare(filePath) == 0)
	{
		// This is the same file, no need to reload
		return;
	}

	// Need to reset buffers
	Reset();

	// Process file
	std::ifstream file(filePath);
	std::string line;
	while (std::getline(file, line))
	{
		ProcessLine((char*)line.c_str());
	}
	file.close();

	if (!normalHandler->normals.size())
	{
		GenerateNormals();
	}

	if (normalHandler->normals.size())
	{
		GenerateTangents();
	}

	currentFile = filePath;
}

void AssetSuite::ModelLoader::GenerateNormals()
{
	unsigned int normalIndicesCount = 0;
	// Generate Normals
	for (unsigned int i = 0; i < faceHandler->faces.size(); i++)
	{
		// Assemble vertices for particular face
		auto p1 = vertexHandler->GetVertexAt(faceHandler->faces[i].vertexIndex[0]);
		auto p2 = vertexHandler->GetVertexAt(faceHandler->faces[i].vertexIndex[1]);
		auto p3 = vertexHandler->GetVertexAt(faceHandler->faces[i].vertexIndex[2]);

		// Calculate two vectors
		auto a = static_cast<MiniMath::Vector3D>(p2 - p1);
		auto b = static_cast<MiniMath::Vector3D>(p3 - p1);

		// Caluclate cross product and normalize
		MiniMath::Vector3D crossProduct = MiniMath::CrossProduct(a, b);
		crossProduct.Normalize();

		// Push new normal to the normals heap
		normalHandler->normals.push_back(crossProduct);

		// Now push to the normal
		for (int j = 0; j < 3; j++)
		{
			faceHandler->faces[i].normalIndex.push_back(normalIndicesCount);
		}
		normalIndicesCount++;
	}
}

void AssetSuite::ModelLoader::GenerateTangents()
{
	unsigned int tangentIndicesCount = 0;
	// Generate Normals
	for (unsigned int i = 0; i < faceHandler->faces.size(); i++)
	{
		// Assemble vertices for particular face
		auto p1 = vertexHandler->GetVertexAt(faceHandler->faces[i].vertexIndex[0]);
		auto p2 = vertexHandler->GetVertexAt(faceHandler->faces[i].vertexIndex[1]);
		auto p3 = vertexHandler->GetVertexAt(faceHandler->faces[i].vertexIndex[2]);

		// Calculate two vectors
		auto a = static_cast<MiniMath::Vector3D>(p2 - p1);
		auto b = static_cast<MiniMath::Vector3D>(p3 - p1);

		// Assemble UVs
		auto uv1 = textureHandler->GetTextureCoordAt(faceHandler->faces[i].textureIndex[0]);
		auto uv2 = textureHandler->GetTextureCoordAt(faceHandler->faces[i].textureIndex[1]);
		auto uv3 = textureHandler->GetTextureCoordAt(faceHandler->faces[i].textureIndex[2]);

		// Calculate UV deltas
		auto deltaUv1 = uv2 - uv1;
		auto deltaUv2 = uv3 - uv1;

		// Calculate tangents
		float f = 1.0f / (deltaUv1.x * deltaUv2.y - deltaUv2.x * deltaUv1.y);

		MiniMath::Vector3D tangent(0.0f, 0.0f, 0.0f);
		tangent.x = f * (deltaUv2.y * a.x - deltaUv1.y / b.x);
		tangent.y = f * (deltaUv2.y * a.y - deltaUv1.y / b.y);
		tangent.z = f * (deltaUv2.y * a.z - deltaUv1.y / b.z);

		// Push the tangent to the tangent heap
		normalHandler->tangents.push_back(tangent);

		// Now push to the normal
		for (int j = 0; j < 3; j++)
		{
			faceHandler->faces[i].tangentIndex.push_back(tangentIndicesCount);
		}
		tangentIndicesCount++;
	}
}

void AssetSuite::ModelLoader::DumpToFile(std::string filename)
{
	std::ofstream file(filename);
	
	for (int i = 0; i < vertexHandler->vertices.size(); i++)
	{
		file << "v " << vertexHandler->vertices[i] << std::endl;
	}

	for (int i = 0; i < normalHandler->normals.size(); i++)
	{
		file << "vn " << normalHandler->normals[i] << std::endl;
	}

	for (int i = 0; i < faceHandler->faces.size(); i++)
	{
		file << "f " << faceHandler->faces[i] << std::endl;
	}

	file.close();
}

void AssetSuite::ModelLoader::ProcessLine(char* lineBuffer)
{
	char* first;
	char* next = NULL;

	std::vector<std::string> tokens;

	// Tokenize
	first = strtok_s(lineBuffer, " ", &next);

	if (!first)
	{
		return;
	}

	do
	{
		tokens.push_back(first);
	} while ((first = strtok_s(NULL, " ", &next)) != NULL);

	// Chain of responsibility
	Handler* current = vertexHandler;
	do
	{
		if (current->Condition(tokens))
		{
			current->Handle(tokens);
			break;
		}
		current = current->next;
	} while (current);
}

Face AssetSuite::ModelLoader::GetFace(unsigned int index)
{
	return faceHandler->faces[index];
}

MiniMath::Point3D AssetSuite::ModelLoader::GetVertex(unsigned int index)
{
	return vertexHandler->vertices[index];
}

MiniMath::Vector3D AssetSuite::ModelLoader::GetNormal(unsigned int index)
{
	return normalHandler->normals[index];
}

MiniMath::Vector3D AssetSuite::ModelLoader::GetTangent(unsigned int index)
{
	return normalHandler->tangents[index];
}

MiniMath::Point2D AssetSuite::ModelLoader::GetTextureCoord(unsigned int index)
{
	return textureHandler->textureCoord[index];
}

unsigned int AssetSuite::ModelLoader::GenNumOfFaces()
{
	return faceHandler->faces.size();
}

unsigned int AssetSuite::ModelLoader::GetGroupOffset(const std::string& key)
{
	return groupHandler->GetGroupOffset(key);
}

unsigned int AssetSuite::ModelLoader::GetGroupSize(const std::string& key)
{
	return groupHandler->GetGroupSize(key);
}

unsigned int AssetSuite::ModelLoader::GetNumOfGroups()
{
	return groupHandler->groupOffset.size();
}

bool AssetSuite::ModelLoader::HasGroup(const std::string& key)
{
	return groupHandler->HasGroup(key);
}

void AssetSuite::ModelLoader::Reset()
{
	// Chain of responsibility
	Handler* current = vertexHandler;
	do
	{
		current->Reset();
		current = current->next;
	} while (current);
}
