#pragma once

#include <AssetSuite/AssetSuiteDescriptors.h>
#include <AssetSuite/AssetSuiteHandles.h>
#include <AssetSuite/AssetSuiteTypes.h>

#include <cstdint>
#include <filesystem>
#include <vector>

namespace AssetSuite::Internal
{
	struct BlobSourceMetadata final
	{
		std::filesystem::path sourcePath;
		std::filesystem::path extension;
		AssetFormat format = AssetFormat::Unknown;
	};

	class Blob final
	{
	public:
		Blob(std::vector<uint8_t> bytes, BlobSourceMetadata metadata);

		Blob(const Blob&) = delete;
		Blob& operator=(const Blob&) = delete;
		Blob(Blob&&) noexcept;
		Blob& operator=(Blob&&) noexcept;

		const uint8_t* Data() const noexcept;
		uint64_t ByteSize() const noexcept;
		const BlobSourceMetadata& SourceMetadata() const noexcept;
		BlobDesc Describe() const noexcept;

	private:
		std::vector<uint8_t> bytes;
		BlobSourceMetadata metadata;
	};

	BlobSourceMetadata MakeBlobSourceMetadata(const std::filesystem::path& sourcePath);
}

namespace AssetSuite
{
	struct AssetSuiteBlob_t
	{
	};
}
