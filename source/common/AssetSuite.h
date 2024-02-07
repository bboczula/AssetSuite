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
		/// <summary>
		/// Loads image from hadr drive, decodes it and stores it in a local memory as consecutive bytes
		/// </summary>
		/// <param name="filePathAndName">File name, extension and path containing image to load</param>
		/// <param name="imageDescriptor">Output structure that will contain necessary image metadata</param>
		/// <returns>A pointer to a decoded image in the local memory as consecutive bytes</returns>
		ErrorCode LoadImageFromFile(const char* filePathAndName, ImageDescriptor& imageDescriptor, std::vector<BYTE>& output);

		// This function would take the buffer that represents the image from local memory
		// And will store them on the hard drive
		// This is the RAW image, meaning it has been stripped from any metadata
		void StoreImageToFile(const std::string& filePathAndName, const std::vector<BYTE>& buffer, const ImageDescriptor& imageDescriptor);

		BYTE* LoadMeshFromFile(const std::string& filePathAndName, MeshDescriptor& meshDescriptor);

		void StoreMeshToFile(const std::string& filePathAndName, BYTE* buffer, const MeshDescriptor& imageDescriptor);

		ErrorCode ImageLoad(const char* filePathAndName);
		ErrorCode ImageDecode(ImageDecoders decoder, ImageDescriptor& descriptor);
		ErrorCode ImageGet(OutputFormat format, std::vector<BYTE>& output);

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