#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <vector>
#include <Windows.h>

#include "../common/ImageDescriptor.h"
#include "../common/ImageDecoder.h"
#include "../common/MeshDecoder.h"
#include "../common/AssetSuite.h"

namespace AssetSuite
{
	class ModelLoader;
	class BmpDecoder;
	class PngDecoder;
	class PpmEncoder;
	class BypassEncoder;
}

namespace AssetSuite::Internal
{
	struct RuntimeState final
	{
		struct FileInfo
		{
			std::filesystem::path fullName;
			std::filesystem::path extension;
		};

		struct ImageInfo
		{
			UINT width = 0;
			UINT height = 0;
			ImageFormat format = ImageFormat::Unknown;
		};

		struct MeshInfo
		{
			UINT numOfVertices = 0;
			UINT numOfIndices = 0;
		};

		RuntimeState();
		~RuntimeState();

		RuntimeState(const RuntimeState&) = delete;
		RuntimeState& operator=(const RuntimeState&) = delete;

		FileInfo fileInfo;
		ImageInfo imageInfo;
		MeshInfo meshInfo;
		std::vector<BYTE> rawBuffer;
		std::vector<BYTE> decodedBuffer;
		std::vector<BYTE> formattedBuffer;
		std::unique_ptr<ModelLoader> modelLoader;
		std::unique_ptr<BmpDecoder> bmpDecoder;
		std::unique_ptr<PngDecoder> pngDecoder;
		std::unique_ptr<PpmEncoder> ppmEncoder;
		std::unique_ptr<BypassEncoder> bypassEncoder;
		std::array<ImageDecoder*, static_cast<size_t>(ImageDecoders::MaxDecoders)> imageDecoders;
		std::array<MeshDecoder*, static_cast<size_t>(MeshDecoders::MaxDecoders)> meshDecoders;
	};
}
