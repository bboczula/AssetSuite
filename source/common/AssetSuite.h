#pragma once

// This is not great, maybe figure out better naming
#ifdef ASSETSUITE_EXPORTS
#define ASSET_SUITE_EXPORTS __declspec(dllexport)
#else
#define ASSET_SUITE_EXPORTS __declspec(dllimport)
#endif

#include <string>
#include <vector>
#include <filesystem>
#include <Windows.h>

#include "ImageDescriptor.h"

// I want to be able to easily extend the interface, so I can add new file fromats
// Or even some custom binary files, or add JPEG or even different 3D formats

namespace AssetSuite
{
	// Forward Declarations
	class ModelLoader;
	class BmpDecoder;
	class PngDecoder;
	class PpmEncoder;
	class BypassEncoder;

	enum class ASSET_SUITE_EXPORTS ImageDecoders
	{
		Auto,
		PNG,
		BMP,
		PPM
	};

	enum class ASSET_SUITE_EXPORTS MeshDecoders
	{
		WAVEFRONT
	};

	enum class ASSET_SUITE_EXPORTS ErrorCode
	{
		OK = 0,
		NonExistingFile = -1,
		FileTypeNotSupported = -2,
		ColorTypeNotSupported = -3,
		Undefined = -1000
	};

	enum class ASSET_SUITE_EXPORTS ErrorCodeLoad
	{
		OK = 0,
		FileNotExist = -1
	};

	enum class ASSET_SUITE_EXPORTS OutputFormat
	{
		RGB8,
		RGBA8
	};

	struct ASSET_SUITE_EXPORTS MeshDescriptor
	{
		UINT numOfVertices;
		UINT numOfIndices;
	};

	class ASSET_SUITE_EXPORTS Manager
	{
	public:
		Manager();
		~Manager();
		
		void StoreImageToFile(const std::string& filePathAndName, const std::vector<BYTE>& buffer, const ImageDescriptor& imageDescriptor);
		ErrorCode ImageLoadAndDecode(const char* filePathAndName, ImageDescriptor& descriptor, ImageDecoders decoder = ImageDecoders::Auto);
		ErrorCode ImageLoad(const char* filePathAndName);
		ErrorCode ImageDecode(ImageDecoders decoder, ImageDescriptor& descriptor);
		ErrorCode ImageGet(OutputFormat format, std::vector<BYTE>& output);

		BYTE* LoadMeshFromFile(const std::string& filePathAndName, MeshDescriptor& meshDescriptor);
		void StoreMeshToFile(const std::string& filePathAndName, BYTE* buffer, const MeshDescriptor& imageDescriptor);
		ErrorCode LoadMesh();
		ErrorCode DecodeMesh();
		ErrorCode GetMesh();
	private:
		struct FileInfo
		{
			std::filesystem::path fullName;
			std::filesystem::path extension;
			bool hasBeenProcessed = false;
		};
		ErrorCode LoadFileToMemory(const std::string& fileName);
		void StoreMemoryToFile(const std::vector<BYTE>& buffer, const std::string& fileName);
		void DumpByteVectorToCpp(const std::vector<BYTE>& byteVector);
		FileInfo fileInfo;
		std::vector<BYTE> rawBuffer;
		std::vector<BYTE> decodedBuffer;
		std::vector<BYTE> formattedBuffer;
		ModelLoader* modelLoader;
		BmpDecoder* bmpDecoder;
		PngDecoder* pngDecoder;
		PpmEncoder* ppmEncoder;
		BypassEncoder* bypassEncoder;
	};
}