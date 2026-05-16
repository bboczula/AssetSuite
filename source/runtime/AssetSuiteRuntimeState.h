#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
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
		struct AllocatorPolicy
		{
			using AllocateCallback = void* (*)(size_t size, size_t alignment, void* userData);
			using FreeCallback = void (*)(void* memory, void* userData);

			AllocateCallback allocate = nullptr;
			FreeCallback free = nullptr;
			void* userData = nullptr;

			void* Allocate(size_t size, size_t alignment);
			void Free(void* memory) noexcept;
		};

		struct Diagnostics
		{
			struct Entry
			{
				ErrorCode code = ErrorCode::OK;
				std::string message;
			};

			void Clear();
			void Add(ErrorCode code, const char* message);
			const std::vector<Entry>& Entries() const noexcept;

		private:
			std::vector<Entry> entries;
		};

		struct FileLoader
		{
			ErrorCode LoadToMemory(const std::filesystem::path& fileName, bool isBinary, std::vector<BYTE>& output) const;
		};

		struct CodecRegistry
		{
			void RegisterImageDecoder(ImageDecoders decoder, ImageDecoder& implementation) noexcept;
			void RegisterMeshDecoder(MeshDecoders decoder, MeshDecoder& implementation) noexcept;
			ImageDecoder* FindImageDecoder(ImageDecoders decoder) const noexcept;
			MeshDecoder* FindMeshDecoder(MeshDecoders decoder) const noexcept;
			ImageDecoders ResolveImageDecoder(const std::filesystem::path& extension) const noexcept;
			MeshDecoders ResolveMeshDecoder(const std::filesystem::path& extension) const noexcept;

		private:
			std::array<ImageDecoder*, static_cast<size_t>(ImageDecoders::MaxDecoders)> imageDecoders = {};
			std::array<MeshDecoder*, static_cast<size_t>(MeshDecoders::MaxDecoders)> meshDecoders = {};
		};

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
		AllocatorPolicy allocatorPolicy;
		Diagnostics diagnostics;
		FileLoader fileLoader;
		CodecRegistry codecRegistry;
	};
}
