#include "AssetSuiteRuntimeState.h"

#include "../wavefront/ModelLoader.h"
#include "../bmp/BmpDecoder.h"
#include "../png/PngDecoder.h"
#include "../ppm/PpmEncoder.h"
#include "../bypass/BypassEncoder.h"

#include <fstream>
#include <limits>
#include <new>
#include <atomic>
#include <cstdint>
#include <system_error>
#include <utility>

namespace
{
	static_assert(sizeof(uintptr_t) >= 8, "BlobHandle token encoding requires a 64-bit target.");

	constexpr uint32_t SLOT_INDEX_BITS = 24;
	constexpr uint32_t GENERATION_BITS = 20;
	constexpr uint32_t CONTEXT_ID_BITS = 20;
	constexpr uintptr_t SLOT_INDEX_MASK = (uintptr_t{ 1 } << SLOT_INDEX_BITS) - 1u;
	constexpr uintptr_t GENERATION_MASK = (uintptr_t{ 1 } << GENERATION_BITS) - 1u;
	constexpr uintptr_t CONTEXT_ID_MASK = (uintptr_t{ 1 } << CONTEXT_ID_BITS) - 1u;
	constexpr uint32_t INITIAL_BLOB_GENERATION = 1;
	constexpr uint32_t INITIAL_BLOB_CONTEXT_ID = 1;

	std::atomic<uint32_t> nextBlobContextId = INITIAL_BLOB_CONTEXT_ID;

	AssetSuite::BlobHandle EncodeBlobHandle(size_t slotIndex, uint32_t generation, uint32_t contextId) noexcept
	{
		if (slotIndex >= SLOT_INDEX_MASK || generation == 0 || contextId == 0)
		{
			return nullptr;
		}

		const uintptr_t token =
			(static_cast<uintptr_t>(contextId) << (SLOT_INDEX_BITS + GENERATION_BITS)) |
			(static_cast<uintptr_t>(generation) << SLOT_INDEX_BITS) |
			(static_cast<uintptr_t>(slotIndex) + 1u);
		return reinterpret_cast<AssetSuite::BlobHandle>(token);
	}

	bool DecodeBlobHandle(
		AssetSuite::BlobHandle handle,
		size_t& slotIndex,
		uint32_t& generation,
		uint32_t& contextId) noexcept
	{
		const uintptr_t token = reinterpret_cast<uintptr_t>(handle);
		const uintptr_t encodedSlotIndex = token & SLOT_INDEX_MASK;
		if (encodedSlotIndex == 0)
		{
			return false;
		}

		generation = static_cast<uint32_t>((token >> SLOT_INDEX_BITS) & GENERATION_MASK);
		contextId = static_cast<uint32_t>((token >> (SLOT_INDEX_BITS + GENERATION_BITS)) & CONTEXT_ID_MASK);
		if (generation == 0 || contextId == 0)
		{
			return false;
		}

		slotIndex = static_cast<size_t>(encodedSlotIndex - 1u);
		return true;
	}

	uint32_t AllocateBlobContextId() noexcept
	{
		uint32_t contextId = nextBlobContextId.fetch_add(1, std::memory_order_relaxed) & static_cast<uint32_t>(CONTEXT_ID_MASK);
		return contextId == 0 ? INITIAL_BLOB_CONTEXT_ID : contextId;
	}

	uint32_t NextBlobGeneration(uint32_t generation) noexcept
	{
		generation = (generation + 1u) & static_cast<uint32_t>(GENERATION_MASK);
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
	std::error_code fileStatusError;
	if (!std::filesystem::exists(fileName, fileStatusError))
	{
		if (fileStatusError)
		{
			return ErrorCode::IoFailure;
		}

		return ErrorCode::NonExistingFile;
	}

	if (!std::filesystem::is_regular_file(fileName, fileStatusError) || fileStatusError)
	{
		return ErrorCode::IoFailure;
	}

	std::ifstream file(fileName.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		return ErrorCode::IoFailure;
	}

	const std::streampos endPosition = file.tellg();
	if (endPosition == std::streampos(-1))
	{
		return ErrorCode::IoFailure;
	}

	file.seekg(0, std::ios::beg);
	if (!file.good())
	{
		return ErrorCode::IoFailure;
	}

	const std::streampos beginPosition = file.tellg();
	if (beginPosition == std::streampos(-1) || endPosition < beginPosition)
	{
		return ErrorCode::IoFailure;
	}

	const std::streamoff size = endPosition - beginPosition;
	if (size > static_cast<std::streamoff>(std::numeric_limits<std::streamsize>::max()) ||
		size > static_cast<std::streamoff>(std::numeric_limits<size_t>::max()))
	{
		return ErrorCode::IoFailure;
	}

	std::vector<uint8_t> loadedBytes;
	if (size > 0)
	{
		loadedBytes.resize(static_cast<size_t>(size));
		file.read(reinterpret_cast<char*>(loadedBytes.data()), static_cast<std::streamsize>(size));
		if (file.gcount() != static_cast<std::streamsize>(size) || !file)
		{
			return ErrorCode::IoFailure;
		}
	}

	if (!isBinary)
	{
		loadedBytes.push_back('\0');
	}

	output = std::move(loadedBytes);
	return ErrorCode::OK;
}

AssetSuite::Internal::RuntimeState::BlobStorage::BlobStorage()
	: contextId(AllocateBlobContextId())
{
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
	return EncodeBlobHandle(slotIndex, slots[slotIndex].generation, contextId);
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
	uint32_t decodedContextId = 0;
	if (!DecodeBlobHandle(blob, slotIndex, generation, decodedContextId))
	{
		return nullptr;
	}

	if (decodedContextId != contextId)
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
	uint32_t decodedContextId = 0;
	if (!blob || !DecodeBlobHandle(*blob, slotIndex, generation, decodedContextId))
	{
		return Result::ErrorInvalidHandle;
	}

	if (decodedContextId != contextId)
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
