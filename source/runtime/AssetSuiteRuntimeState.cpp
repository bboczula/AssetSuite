#include "AssetSuiteRuntimeState.h"

#include "../wavefront/ModelLoader.h"
#include "../bmp/BmpDecoder.h"
#include "../png/PngDecoder.h"
#include "../ppm/PpmEncoder.h"
#include "../bypass/BypassEncoder.h"

#include <fstream>
#include <new>
#include <cstdint>
#include <utility>

namespace
{
	constexpr uintptr_t SLOT_INDEX_MASK = 0xffffffffu;
	constexpr uint32_t INITIAL_BLOB_GENERATION = 1;

	AssetSuite::BlobHandle EncodeBlobHandle(size_t slotIndex, uint32_t generation) noexcept
	{
		const uintptr_t token =
			(static_cast<uintptr_t>(generation) << 32) |
			(static_cast<uintptr_t>(slotIndex) + 1u);
		return reinterpret_cast<AssetSuite::BlobHandle>(token);
	}

	bool DecodeBlobHandle(AssetSuite::BlobHandle handle, size_t& slotIndex, uint32_t& generation) noexcept
	{
		const uintptr_t token = reinterpret_cast<uintptr_t>(handle);
		const uintptr_t encodedSlotIndex = token & SLOT_INDEX_MASK;
		if (encodedSlotIndex == 0)
		{
			return false;
		}

		generation = static_cast<uint32_t>(token >> 32);
		if (generation == 0)
		{
			return false;
		}

		slotIndex = static_cast<size_t>(encodedSlotIndex - 1u);
		return true;
	}

	uint32_t NextBlobGeneration(uint32_t generation) noexcept
	{
		++generation;
		return generation == 0 ? INITIAL_BLOB_GENERATION : generation;
	}
}

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
	std::vector<uint8_t>& output) const
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
	size_t slotIndex = 0;
	if (!freeSlots.empty())
	{
		slotIndex = freeSlots.back();
		freeSlots.pop_back();
	}
	else
	{
		slotIndex = slots.size();
		slots.push_back({ nullptr, INITIAL_BLOB_GENERATION });
	}

	slots[slotIndex].blob = std::make_unique<Blob>(std::move(blob));
	return EncodeBlobHandle(slotIndex, slots[slotIndex].generation);
}

bool AssetSuite::Internal::RuntimeState::BlobStorage::Owns(BlobHandle blob) const noexcept
{
	return Get(blob) != nullptr;
}

bool AssetSuite::Internal::RuntimeState::BlobStorage::IsLive(BlobHandle blob) const noexcept
{
	return Get(blob) != nullptr;
}

const AssetSuite::Internal::Blob*
AssetSuite::Internal::RuntimeState::BlobStorage::Get(BlobHandle blob) const noexcept
{
	size_t slotIndex = 0;
	uint32_t generation = 0;
	if (!DecodeBlobHandle(blob, slotIndex, generation))
	{
		return nullptr;
	}

	if (slotIndex >= slots.size())
	{
		return nullptr;
	}

	const Slot& slot = slots[slotIndex];
	if (slot.generation != generation || !slot.blob)
	{
		return nullptr;
	}

	return slot.blob.get();
}

AssetSuite::Result AssetSuite::Internal::RuntimeState::BlobStorage::Release(BlobHandle* blob) noexcept
{
	size_t slotIndex = 0;
	uint32_t generation = 0;
	if (!blob || !DecodeBlobHandle(*blob, slotIndex, generation))
	{
		return Result::ErrorInvalidHandle;
	}

	if (slotIndex >= slots.size())
	{
		return Result::ErrorInvalidHandle;
	}

	Slot& slot = slots[slotIndex];
	if (slot.generation != generation || !slot.blob)
	{
		return Result::ErrorInvalidHandle;
	}

	slot.blob.reset();
	slot.generation = NextBlobGeneration(slot.generation);
	freeSlots.push_back(slotIndex);
	*blob = nullptr;
	return Result::Success;
}

size_t AssetSuite::Internal::RuntimeState::BlobStorage::LiveCount() const noexcept
{
	size_t count = 0;
	for (const auto& slot : slots)
	{
		if (slot.blob)
		{
			++count;
		}
	}

	return count;
}

size_t AssetSuite::Internal::RuntimeState::BlobStorage::SlotCapacity() const noexcept
{
	return slots.size();
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
