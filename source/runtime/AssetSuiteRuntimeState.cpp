#include "AssetSuiteRuntimeState.h"

#include "../wavefront/ModelLoader.h"
#include "../bmp/BmpDecoder.h"
#include "../png/PngDecoder.h"
#include "../ppm/PpmEncoder.h"
#include "../bypass/BypassEncoder.h"

#include <fstream>
#include <new>
#include <utility>

AssetSuite::Internal::RuntimeState::RuntimeState()
	: codecs()
{
	codecs.RegisterWith(codecRegistry);
}

AssetSuite::Internal::RuntimeState::~RuntimeState() = default;

AssetSuite::Internal::RuntimeState::CodecStorage::CodecStorage()
	: modelLoader(std::make_unique<ModelLoader>())
	, bmpDecoder(std::make_unique<BmpDecoder>())
	, pngDecoder(std::make_unique<PngDecoder>())
	, ppmEncoder(std::make_unique<PpmEncoder>())
	, bypassEncoder(std::make_unique<BypassEncoder>())
{
}

AssetSuite::Internal::RuntimeState::CodecStorage::~CodecStorage() = default;

void AssetSuite::Internal::RuntimeState::CodecStorage::RegisterWith(CodecRegistry& registry) noexcept
{
	registry.RegisterImageDecoder(ImageDecoders::BMP, *bmpDecoder);
	registry.RegisterImageDecoder(ImageDecoders::PNG, *pngDecoder);
	registry.RegisterMeshDecoder(MeshDecoders::WAVEFRONT, *modelLoader);
}

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

AssetSuite::BlobHandle AssetSuite::Internal::RuntimeState::BlobStorage::Create(Blob blob)
{
	auto storedBlob = std::make_unique<AssetSuiteBlob_t>(std::move(blob));
	BlobHandle handle = storedBlob.get();
	blobs.push_back(std::move(storedBlob));
	return handle;
}

bool AssetSuite::Internal::RuntimeState::BlobStorage::Owns(BlobHandle blob) const noexcept
{
	for (const auto& storedBlob : blobs)
	{
		if (storedBlob.get() == blob)
		{
			return true;
		}
	}

	return false;
}

bool AssetSuite::Internal::RuntimeState::BlobStorage::IsLive(BlobHandle blob) const noexcept
{
	return Owns(blob) && blob->IsLive();
}

AssetSuite::Result AssetSuite::Internal::RuntimeState::BlobStorage::Release(BlobHandle* blob) noexcept
{
	if (!blob || !*blob || !IsLive(*blob))
	{
		return Result::ErrorInvalidHandle;
	}

	(*blob)->Release();
	*blob = nullptr;
	return Result::Success;
}

size_t AssetSuite::Internal::RuntimeState::BlobStorage::LiveCount() const noexcept
{
	size_t count = 0;
	for (const auto& storedBlob : blobs)
	{
		if (storedBlob->IsLive())
		{
			++count;
		}
	}

	return count;
}

bool AssetSuite::Internal::RuntimeState::CodecRegistry::RegisterImageDecoder(
	ImageDecoders decoder,
	ImageDecoder& implementation) noexcept
{
	if (decoder == ImageDecoders::Auto || decoder == ImageDecoders::MaxDecoders)
	{
		return false;
	}

	imageDecoders[static_cast<size_t>(decoder)] = &implementation;
	return true;
}

bool AssetSuite::Internal::RuntimeState::CodecRegistry::RegisterMeshDecoder(
	MeshDecoders decoder,
	MeshDecoder& implementation) noexcept
{
	if (decoder == MeshDecoders::Auto || decoder == MeshDecoders::MaxDecoders)
	{
		return false;
	}

	meshDecoders[static_cast<size_t>(decoder)] = &implementation;
	return true;
}

AssetSuite::ImageDecoder* AssetSuite::Internal::RuntimeState::CodecRegistry::FindImageDecoder(
	ImageDecoders decoder) const noexcept
{
	if (decoder == ImageDecoders::Auto || decoder == ImageDecoders::MaxDecoders)
	{
		return nullptr;
	}

	return imageDecoders[static_cast<size_t>(decoder)];
}

AssetSuite::MeshDecoder* AssetSuite::Internal::RuntimeState::CodecRegistry::FindMeshDecoder(
	MeshDecoders decoder) const noexcept
{
	if (decoder == MeshDecoders::Auto || decoder == MeshDecoders::MaxDecoders)
	{
		return nullptr;
	}

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
