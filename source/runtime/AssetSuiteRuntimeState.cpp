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
#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
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

	std::filesystem::path NormalizeExtension(const std::filesystem::path& extension)
	{
		std::string normalizedExtension = extension.string();
		std::transform(
			normalizedExtension.begin(),
			normalizedExtension.end(),
			normalizedExtension.begin(),
			[](unsigned char character)
			{
				return static_cast<char>(std::tolower(character));
			});

		return std::filesystem::path(normalizedExtension);
	}

	bool ExtensionMatches(
		const std::vector<std::filesystem::path>& supportedExtensions,
		const std::filesystem::path& extension)
	{
		const std::filesystem::path normalizedExtension = NormalizeExtension(extension);
		return std::find(supportedExtensions.begin(), supportedExtensions.end(), normalizedExtension) != supportedExtensions.end();
	}

	bool HasBmpSignature(const uint8_t* data, size_t size) noexcept
	{
		return data && size >= 2 && data[0] == 'B' && data[1] == 'M';
	}

	bool HasPngSignature(const uint8_t* data, size_t size) noexcept
	{
		constexpr uint8_t PNG_SIGNATURE[] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
		return data && size >= sizeof(PNG_SIGNATURE) && std::memcmp(data, PNG_SIGNATURE, sizeof(PNG_SIGNATURE)) == 0;
	}

	bool StartsWithToken(const char* line, size_t lineLength, const char* token) noexcept
	{
		const size_t tokenLength = std::strlen(token);
		return lineLength >= tokenLength &&
			std::memcmp(line, token, tokenLength) == 0 &&
			(lineLength == tokenLength || std::isspace(static_cast<unsigned char>(line[tokenLength])) != 0);
	}

	bool HasWavefrontObjContent(const uint8_t* data, size_t size) noexcept
	{
		if (!data || size == 0)
		{
			return false;
		}

		const char* text = reinterpret_cast<const char*>(data);
		size_t offset = 0;
		bool hasVertex = false;
		bool hasFace = false;
		while (offset < size)
		{
			while (offset < size && (text[offset] == ' ' || text[offset] == '\t' || text[offset] == '\r' || text[offset] == '\n'))
			{
				++offset;
			}

			if (offset >= size || text[offset] == '\0')
			{
				break;
			}

			const size_t lineStart = offset;
			while (offset < size && text[offset] != '\r' && text[offset] != '\n' && text[offset] != '\0')
			{
				++offset;
			}

			const size_t lineLength = offset - lineStart;
			const char* line = text + lineStart;
			if (StartsWithToken(line, lineLength, "v"))
			{
				hasVertex = true;
			}
			else if (StartsWithToken(line, lineLength, "f"))
			{
				hasFace = true;
			}

			if (hasVertex && hasFace)
			{
				return true;
			}
		}

		return false;
	}

	AssetSuite::AssetFormat FormatForImageDecoder(AssetSuite::ImageDecoders decoder) noexcept
	{
		switch (decoder)
		{
		case AssetSuite::ImageDecoders::BMP:
			return AssetSuite::AssetFormat::BMP;
		case AssetSuite::ImageDecoders::PNG:
			return AssetSuite::AssetFormat::PNG;
		default:
			return AssetSuite::AssetFormat::Unknown;
		}
	}

	std::vector<std::filesystem::path> ExtensionsForImageDecoder(AssetSuite::ImageDecoders decoder)
	{
		switch (decoder)
		{
		case AssetSuite::ImageDecoders::BMP:
			return { ".bmp" };
		case AssetSuite::ImageDecoders::PNG:
			return { ".png" };
		default:
			return {};
		}
	}

	AssetSuite::Internal::RuntimeState::CodecRegistry::ProbeCallback ProbeForImageDecoder(
		AssetSuite::ImageDecoders decoder) noexcept
	{
		switch (decoder)
		{
		case AssetSuite::ImageDecoders::BMP:
			return &HasBmpSignature;
		case AssetSuite::ImageDecoders::PNG:
			return &HasPngSignature;
		default:
			return nullptr;
		}
	}

	AssetSuite::AssetFormat FormatForMeshDecoder(AssetSuite::MeshDecoders decoder) noexcept
	{
		switch (decoder)
		{
		case AssetSuite::MeshDecoders::WAVEFRONT:
			return AssetSuite::AssetFormat::WavefrontObj;
		default:
			return AssetSuite::AssetFormat::Unknown;
		}
	}

	std::vector<std::filesystem::path> ExtensionsForMeshDecoder(AssetSuite::MeshDecoders decoder)
	{
		switch (decoder)
		{
		case AssetSuite::MeshDecoders::WAVEFRONT:
			return { ".obj" };
		default:
			return {};
		}
	}

	AssetSuite::Internal::RuntimeState::CodecRegistry::ProbeCallback ProbeForMeshDecoder(
		AssetSuite::MeshDecoders decoder) noexcept
	{
		switch (decoder)
		{
		case AssetSuite::MeshDecoders::WAVEFRONT:
			return &HasWavefrontObjContent;
		default:
			return nullptr;
		}
	}

	std::vector<std::filesystem::path> ExtensionsForImageEncoder(AssetSuite::AssetFormat format)
	{
		switch (format)
		{
		case AssetSuite::AssetFormat::PPM:
			return { ".ppm" };
		default:
			return {};
		}
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

void AssetSuite::Internal::RuntimeState::CodecStorage::RegisterWith(CodecRegistry& registry)
{
	registry.RegisterImageDecoder(ImageDecoders::BMP, *bmpDecoder);
	registry.RegisterImageDecoder(ImageDecoders::PNG, *pngDecoder);
	registry.RegisterMeshDecoder(MeshDecoders::WAVEFRONT, *modelLoader);
	registry.RegisterImageEncoder(AssetFormat::PPM, *ppmEncoder);
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
	const auto unsignedSize = static_cast<uintmax_t>(size);
	if (unsignedSize > static_cast<uintmax_t>((std::numeric_limits<std::streamsize>::max)()) ||
		unsignedSize > static_cast<uintmax_t>((std::numeric_limits<size_t>::max)()))
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

AssetSuite::ImageHandle AssetSuite::Internal::RuntimeState::ImageStorage::Create(
	ImageDesc desc,
	std::vector<uint8_t> bytes)
{
	auto image = std::make_unique<AssetSuiteImage_t>();
	image->desc = desc;
	image->bytes = std::move(bytes);

	ImageHandle handle = image.get();
	images.push_back(std::move(image));
	return handle;
}

bool AssetSuite::Internal::RuntimeState::ImageStorage::Owns(ImageHandle image) const noexcept
{
	return Get(image) != nullptr;
}

const AssetSuite::AssetSuiteImage_t*
AssetSuite::Internal::RuntimeState::ImageStorage::Get(ImageHandle image) const noexcept
{
	if (!image)
	{
		return nullptr;
	}

	for (const auto& storedImage : images)
	{
		if (storedImage.get() == image)
		{
			return storedImage.get();
		}
	}

	return nullptr;
}

AssetSuite::Result AssetSuite::Internal::RuntimeState::ImageStorage::Release(ImageHandle* image) noexcept
{
	if (!image || !*image)
	{
		return Result::ErrorInvalidHandle;
	}

	for (auto storedImage = images.begin(); storedImage != images.end(); ++storedImage)
	{
		if (storedImage->get() == *image)
		{
			images.erase(storedImage);
			*image = nullptr;
			return Result::Success;
		}
	}

	return Result::ErrorInvalidHandle;
}

size_t AssetSuite::Internal::RuntimeState::ImageStorage::LiveCount() const noexcept
{
	return images.size();
}

AssetSuite::MeshHandle AssetSuite::Internal::RuntimeState::MeshStorage::Create(
	MeshDesc desc,
	std::vector<uint8_t> bytes)
{
	auto mesh = std::make_unique<AssetSuiteMesh_t>();
	mesh->desc = desc;
	mesh->bytes = std::move(bytes);

	MeshHandle handle = mesh.get();
	meshes.push_back(std::move(mesh));
	return handle;
}

bool AssetSuite::Internal::RuntimeState::MeshStorage::Owns(MeshHandle mesh) const noexcept
{
	return Get(mesh) != nullptr;
}

const AssetSuite::AssetSuiteMesh_t*
AssetSuite::Internal::RuntimeState::MeshStorage::Get(MeshHandle mesh) const noexcept
{
	if (!mesh)
	{
		return nullptr;
	}

	for (const auto& storedMesh : meshes)
	{
		if (storedMesh.get() == mesh)
		{
			return storedMesh.get();
		}
	}

	return nullptr;
}

AssetSuite::Result AssetSuite::Internal::RuntimeState::MeshStorage::Release(MeshHandle* mesh) noexcept
{
	if (!mesh || !*mesh)
	{
		return Result::ErrorInvalidHandle;
	}

	for (auto storedMesh = meshes.begin(); storedMesh != meshes.end(); ++storedMesh)
	{
		if (storedMesh->get() == *mesh)
		{
			meshes.erase(storedMesh);
			*mesh = nullptr;
			return Result::Success;
		}
	}

	return Result::ErrorInvalidHandle;
}

size_t AssetSuite::Internal::RuntimeState::MeshStorage::LiveCount() const noexcept
{
	return meshes.size();
}

bool AssetSuite::Internal::RuntimeState::CodecRegistry::RegisterImageDecoder(
	ImageDecoders decoder,
	ImageDecoder& implementation)
{
	if (decoder == ImageDecoders::Auto || decoder == ImageDecoders::MaxDecoders)
	{
		return false;
	}

	imageDecoders[static_cast<size_t>(decoder)] = &implementation;
	records.push_back(
		{
			CodecRegistry::AssetKind::Image,
			static_cast<uint32_t>(CodecRegistry::Capability::Decode),
			FormatForImageDecoder(decoder),
			decoder,
			MeshDecoders::Auto,
			ExtensionsForImageDecoder(decoder),
			ProbeForImageDecoder(decoder),
			&implementation,
			nullptr,
			nullptr
		});
	return true;
}

bool AssetSuite::Internal::RuntimeState::CodecRegistry::RegisterMeshDecoder(
	MeshDecoders decoder,
	MeshDecoder& implementation)
{
	if (decoder == MeshDecoders::Auto || decoder == MeshDecoders::MaxDecoders)
	{
		return false;
	}

	meshDecoders[static_cast<size_t>(decoder)] = &implementation;
	records.push_back(
		{
			CodecRegistry::AssetKind::Mesh,
			static_cast<uint32_t>(CodecRegistry::Capability::Decode),
			FormatForMeshDecoder(decoder),
			ImageDecoders::Auto,
			decoder,
			ExtensionsForMeshDecoder(decoder),
			ProbeForMeshDecoder(decoder),
			nullptr,
			&implementation,
			nullptr
		});
	return true;
}

bool AssetSuite::Internal::RuntimeState::CodecRegistry::RegisterImageEncoder(
	AssetFormat format,
	ImageEncoder& implementation)
{
	if (format == AssetFormat::Unknown)
	{
		return false;
	}

	records.push_back(
		{
			CodecRegistry::AssetKind::Image,
			static_cast<uint32_t>(CodecRegistry::Capability::Encode),
			format,
			ImageDecoders::Auto,
			MeshDecoders::Auto,
			ExtensionsForImageEncoder(format),
			nullptr,
			nullptr,
			nullptr,
			&implementation
		});
	return true;
}

AssetSuite::ImageDecoder* AssetSuite::Internal::RuntimeState::CodecRegistry::FindImageDecoder(
	ImageDecoders decoder) const noexcept
{
	if (decoder == ImageDecoders::Auto || decoder == ImageDecoders::MaxDecoders)
	{
		return nullptr;
	}

	const CodecRecord* record = FindImageDecoderRecord(decoder);
	return record ? record->imageDecoderImplementation : nullptr;
}

AssetSuite::MeshDecoder* AssetSuite::Internal::RuntimeState::CodecRegistry::FindMeshDecoder(
	MeshDecoders decoder) const noexcept
{
	if (decoder == MeshDecoders::Auto || decoder == MeshDecoders::MaxDecoders)
	{
		return nullptr;
	}

	const CodecRecord* record = FindMeshDecoderRecord(decoder);
	return record ? record->meshDecoderImplementation : nullptr;
}

AssetSuite::ImageEncoder* AssetSuite::Internal::RuntimeState::CodecRegistry::FindImageEncoder(
	AssetFormat format) const noexcept
{
	const CodecRecord* record = FindImageEncoderRecord(format);
	return record ? record->imageEncoderImplementation : nullptr;
}

AssetSuite::ImageDecoders AssetSuite::Internal::RuntimeState::CodecRegistry::ResolveImageDecoder(
	const std::filesystem::path& extension) const noexcept
{
	const std::filesystem::path normalizedExtension = NormalizeExtension(extension);
	for (const CodecRecord& record : records)
	{
		if (record.assetKind == CodecRegistry::AssetKind::Image &&
			(record.capabilities & static_cast<uint32_t>(CodecRegistry::Capability::Decode)) != 0 &&
			ExtensionMatches(record.extensions, normalizedExtension))
		{
			return record.imageDecoder;
		}
	}

	return ImageDecoders::Auto;
}

AssetSuite::MeshDecoders AssetSuite::Internal::RuntimeState::CodecRegistry::ResolveMeshDecoder(
	const std::filesystem::path& extension) const noexcept
{
	const std::filesystem::path normalizedExtension = NormalizeExtension(extension);
	for (const CodecRecord& record : records)
	{
		if (record.assetKind == CodecRegistry::AssetKind::Mesh &&
			(record.capabilities & static_cast<uint32_t>(CodecRegistry::Capability::Decode)) != 0 &&
			ExtensionMatches(record.extensions, normalizedExtension))
		{
			return record.meshDecoder;
		}
	}

	return MeshDecoders::Auto;
}

AssetSuite::ImageDecoders AssetSuite::Internal::RuntimeState::CodecRegistry::ProbeImageDecoder(
	const std::filesystem::path& extension,
	const uint8_t* data,
	size_t size) const noexcept
{
	for (const CodecRecord& record : records)
	{
		if (record.assetKind == CodecRegistry::AssetKind::Image &&
			(record.capabilities & static_cast<uint32_t>(CodecRegistry::Capability::Decode)) != 0 &&
			record.probe &&
			record.probe(data, size))
		{
			return record.imageDecoder;
		}
	}

	return ResolveImageDecoder(extension);
}

AssetSuite::MeshDecoders AssetSuite::Internal::RuntimeState::CodecRegistry::ProbeMeshDecoder(
	const std::filesystem::path& extension,
	const uint8_t* data,
	size_t size) const noexcept
{
	for (const CodecRecord& record : records)
	{
		if (record.assetKind == CodecRegistry::AssetKind::Mesh &&
			(record.capabilities & static_cast<uint32_t>(CodecRegistry::Capability::Decode)) != 0 &&
			record.probe &&
			record.probe(data, size))
		{
			return record.meshDecoder;
		}
	}

	return ResolveMeshDecoder(extension);
}

const std::vector<AssetSuite::Internal::RuntimeState::CodecRegistry::CodecRecord>&
AssetSuite::Internal::RuntimeState::CodecRegistry::Records() const noexcept
{
	return records;
}

const AssetSuite::Internal::RuntimeState::CodecRegistry::CodecRecord*
AssetSuite::Internal::RuntimeState::CodecRegistry::FindImageDecoderRecord(ImageDecoders decoder) const noexcept
{
	if (decoder == ImageDecoders::Auto || decoder == ImageDecoders::MaxDecoders)
	{
		return nullptr;
	}

	for (auto record = records.rbegin(); record != records.rend(); ++record)
	{
		if (record->assetKind == CodecRegistry::AssetKind::Image &&
			(record->capabilities & static_cast<uint32_t>(CodecRegistry::Capability::Decode)) != 0 &&
			record->imageDecoder == decoder)
		{
			return &(*record);
		}
	}

	return nullptr;
}

const AssetSuite::Internal::RuntimeState::CodecRegistry::CodecRecord*
AssetSuite::Internal::RuntimeState::CodecRegistry::FindMeshDecoderRecord(MeshDecoders decoder) const noexcept
{
	if (decoder == MeshDecoders::Auto || decoder == MeshDecoders::MaxDecoders)
	{
		return nullptr;
	}

	for (auto record = records.rbegin(); record != records.rend(); ++record)
	{
		if (record->assetKind == CodecRegistry::AssetKind::Mesh &&
			(record->capabilities & static_cast<uint32_t>(CodecRegistry::Capability::Decode)) != 0 &&
			record->meshDecoder == decoder)
		{
			return &(*record);
		}
	}

	return nullptr;
}

const AssetSuite::Internal::RuntimeState::CodecRegistry::CodecRecord*
AssetSuite::Internal::RuntimeState::CodecRegistry::FindImageEncoderRecord(AssetFormat format) const noexcept
{
	if (format == AssetFormat::Unknown)
	{
		return nullptr;
	}

	for (auto record = records.rbegin(); record != records.rend(); ++record)
	{
		if (record->assetKind == CodecRegistry::AssetKind::Image &&
			(record->capabilities & static_cast<uint32_t>(CodecRegistry::Capability::Encode)) != 0 &&
			record->format == format)
		{
			return &(*record);
		}
	}

	return nullptr;
}
