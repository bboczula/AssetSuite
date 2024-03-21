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
#include "ImageDecoder.h"
#include "MeshDescriptor.h"
#include "MeshDecoder.h"

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
		PPM,
		MaxDecoders
	};

	enum class ASSET_SUITE_EXPORTS MeshDecoders
	{
		Auto,
		WAVEFRONT,
		MaxDecoders
	};

	enum class ASSET_SUITE_EXPORTS ErrorCode
	{
		OK = 0,
		NonExistingFile = -1,
		FileTypeNotSupported = -2,
		ColorTypeNotSupported = -3,
		RawBufferIsEmpty = -4,
		DecodedBufferIsEmpty = -5,
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

	enum class ASSET_SUITE_EXPORTS MeshOutputFormat
	{
		POSITION,
		NORMAL,
		TANGENT,
		TEXCOORD
	};

	class ASSET_SUITE_EXPORTS Manager
	{
	public:
		Manager();
		~Manager();
		
		void StoreImageToFile(const std::string& filePathAndName, const std::vector<BYTE>& buffer, const ImageDescriptor& imageDescriptor);
		ErrorCode ImageLoadAndDecode(const char* filePathAndName, ImageDecoders decoder = ImageDecoders::Auto);
		ErrorCode ImageLoad(const char* filePathAndName);
		ErrorCode ImageDecode(ImageDecoders decoder);
		ErrorCode ImageGet(OutputFormat format, std::vector<BYTE>& output, ImageDescriptor& descriptor);

		void StoreMeshToFile(const std::string& filePathAndName, BYTE* buffer, const MeshDescriptor& imageDescriptor);
		ErrorCode MeshLoadAndDecode(const char* filePathAndName, MeshDecoders decoder = MeshDecoders::Auto);
		ErrorCode MeshLoad(const char* filePathAndName);
		ErrorCode MeshDecode(MeshDecoders decoder);
		ErrorCode MeshGet(const char* meshName, MeshOutputFormat format, std::vector<FLOAT>& output, MeshDescriptor& descriptor);

		ErrorCode DumpRawBuffer();
		ErrorCode DumpDecodedBuffer();
	private:
		struct FileInfo
		{
			std::filesystem::path fullName;
			std::filesystem::path extension;
		};
		struct ImageInfo
		{
			UINT width;
			UINT height;
			ImageFormat format;
		};
		struct MeshInfo
		{
			UINT numOfVertices;
			UINT numOfIndices;
		};
		ErrorCode LoadFileToMemory(const std::string& fileName, bool isBinary = true);
		void StoreMemoryToFile(const std::vector<BYTE>& buffer, const std::string& fileName);
		void DumpByteVectorToCpp(const std::vector<BYTE>& byteVector);
		void DumpBuffer(const std::string& fileName, const std::vector<BYTE>& buffer, ImageDescriptor& descriptor);
		FileInfo fileInfo;
		ImageInfo imageInfo;
		MeshInfo meshInfo;
		std::vector<BYTE> rawBuffer;
		std::vector<BYTE> decodedBuffer;
		std::vector<BYTE> formattedBuffer;
		ModelLoader* modelLoader;
		BmpDecoder* bmpDecoder;
		PngDecoder* pngDecoder;
		PpmEncoder* ppmEncoder;
		BypassEncoder* bypassEncoder;
		ImageDecoder* imageDecoders[(size_t)ImageDecoders::MaxDecoders];
		MeshDecoder* meshDecoders[(size_t)MeshDecoders::MaxDecoders];
	};
}