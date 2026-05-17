#pragma once

#include <string>
#include <vector>
#include <Windows.h>

#include <AssetSuite/AssetSuite.h>

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
	namespace Internal
	{
		struct RuntimeState;
	}

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
		IoFailure = -6,
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
		explicit Manager(Internal::RuntimeState& runtimeState);
		~Manager();

		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;
		
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
		Internal::RuntimeState& State();
		const Internal::RuntimeState& State() const;

		ErrorCode LoadFileToMemory(const std::string& fileName, bool isBinary = true);
		void StoreMemoryToFile(const std::vector<BYTE>& buffer, const std::string& fileName);
		void DumpByteVectorToCpp(const std::vector<BYTE>& byteVector);
		void DumpBuffer(const std::string& fileName, const std::vector<BYTE>& buffer, ImageDescriptor& descriptor);

		Internal::RuntimeState* runtimeState;
		bool ownsRuntimeState;
	};
}
