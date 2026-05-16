#include "AssetSuiteRuntimeState.h"

#include "../wavefront/ModelLoader.h"
#include "../bmp/BmpDecoder.h"
#include "../png/PngDecoder.h"
#include "../ppm/PpmEncoder.h"
#include "../bypass/BypassEncoder.h"

#include <fstream>
#include <new>

AssetSuite::Internal::RuntimeState::RuntimeState()
	: modelLoader(std::make_unique<ModelLoader>())
	, bmpDecoder(std::make_unique<BmpDecoder>())
	, pngDecoder(std::make_unique<PngDecoder>())
	, ppmEncoder(std::make_unique<PpmEncoder>())
	, bypassEncoder(std::make_unique<BypassEncoder>())
{
	codecRegistry.RegisterImageDecoder(ImageDecoders::BMP, *bmpDecoder);
	codecRegistry.RegisterImageDecoder(ImageDecoders::PNG, *pngDecoder);

	codecRegistry.RegisterMeshDecoder(MeshDecoders::WAVEFRONT, *modelLoader);
}

AssetSuite::Internal::RuntimeState::~RuntimeState() = default;

void* AssetSuite::Internal::RuntimeState::AllocatorPolicy::Allocate(size_t size, size_t alignment)
{
	if (allocate)
	{
		return allocate(size, alignment, userData);
	}

	return ::operator new(size);
}

void AssetSuite::Internal::RuntimeState::AllocatorPolicy::Free(void* memory) noexcept
{
	if (!memory)
	{
		return;
	}

	if (free)
	{
		free(memory, userData);
		return;
	}

	::operator delete(memory);
}

void AssetSuite::Internal::RuntimeState::Diagnostics::Clear()
{
	entries.clear();
}

void AssetSuite::Internal::RuntimeState::Diagnostics::Add(ErrorCode code, const char* message)
{
	entries.push_back({ code, message ? message : "" });
}

const std::vector<AssetSuite::Internal::RuntimeState::Diagnostics::Entry>&
AssetSuite::Internal::RuntimeState::Diagnostics::Entries() const noexcept
{
	return entries;
}

AssetSuite::ErrorCode AssetSuite::Internal::RuntimeState::FileLoader::LoadToMemory(
	const std::filesystem::path& fileName,
	bool isBinary,
	std::vector<BYTE>& output) const
{
	if (!std::filesystem::exists(fileName))
	{
		return ErrorCode::NonExistingFile;
	}

	output.clear();

	std::ifstream file(fileName.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
	std::streamsize size = 0;

	if (file.seekg(0, std::ios::end).good())
	{
		size = file.tellg();
	}

	if (file.seekg(0, std::ios::beg).good())
	{
		size -= file.tellg();
	}

	if (size > 0)
	{
		output.resize(static_cast<size_t>(size));
		file.read(reinterpret_cast<char*>(output.data()), size);
		if (!isBinary)
		{
			output.push_back('\0');
		}
	}

	return ErrorCode::OK;
}

void AssetSuite::Internal::RuntimeState::CodecRegistry::RegisterImageDecoder(
	ImageDecoders decoder,
	ImageDecoder& implementation) noexcept
{
	imageDecoders[static_cast<size_t>(decoder)] = &implementation;
}

void AssetSuite::Internal::RuntimeState::CodecRegistry::RegisterMeshDecoder(
	MeshDecoders decoder,
	MeshDecoder& implementation) noexcept
{
	meshDecoders[static_cast<size_t>(decoder)] = &implementation;
}

AssetSuite::ImageDecoder* AssetSuite::Internal::RuntimeState::CodecRegistry::FindImageDecoder(
	ImageDecoders decoder) const noexcept
{
	return imageDecoders[static_cast<size_t>(decoder)];
}

AssetSuite::MeshDecoder* AssetSuite::Internal::RuntimeState::CodecRegistry::FindMeshDecoder(
	MeshDecoders decoder) const noexcept
{
	return meshDecoders[static_cast<size_t>(decoder)];
}

AssetSuite::ImageDecoders AssetSuite::Internal::RuntimeState::CodecRegistry::ResolveImageDecoder(
	const std::filesystem::path& extension) const noexcept
{
	if (extension.compare(".bmp") == 0)
	{
		return ImageDecoders::BMP;
	}

	if (extension.compare(".png") == 0)
	{
		return ImageDecoders::PNG;
	}

	return ImageDecoders::Auto;
}

AssetSuite::MeshDecoders AssetSuite::Internal::RuntimeState::CodecRegistry::ResolveMeshDecoder(
	const std::filesystem::path& extension) const noexcept
{
	if (extension.compare(".obj") == 0)
	{
		return MeshDecoders::WAVEFRONT;
	}

	return MeshDecoders::Auto;
}
