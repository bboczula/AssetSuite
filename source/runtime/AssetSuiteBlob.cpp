#include "AssetSuiteBlob.h"

#include <utility>

namespace
{
	AssetSuite::AssetFormat ResolveBlobFormat(const std::filesystem::path& extension) noexcept
	{
		if (extension == ".bmp")
		{
			return AssetSuite::AssetFormat::BMP;
		}

		if (extension == ".png")
		{
			return AssetSuite::AssetFormat::PNG;
		}

		if (extension == ".ppm")
		{
			return AssetSuite::AssetFormat::PPM;
		}

		if (extension == ".obj")
		{
			return AssetSuite::AssetFormat::WavefrontObj;
		}

		return AssetSuite::AssetFormat::Unknown;
	}
}

AssetSuite::Internal::Blob::Blob(std::vector<uint8_t> bytes, BlobSourceMetadata metadata)
	: bytes(std::move(bytes))
	, metadata(std::move(metadata))
{
}

AssetSuite::Internal::Blob::Blob(Blob&&) noexcept = default;

AssetSuite::Internal::Blob& AssetSuite::Internal::Blob::operator=(Blob&&) noexcept = default;

const uint8_t* AssetSuite::Internal::Blob::Data() const noexcept
{
	return bytes.empty() ? nullptr : bytes.data();
}

uint64_t AssetSuite::Internal::Blob::ByteSize() const noexcept
{
	return static_cast<uint64_t>(bytes.size());
}

const AssetSuite::Internal::BlobSourceMetadata&
AssetSuite::Internal::Blob::SourceMetadata() const noexcept
{
	return metadata;
}

AssetSuite::BlobDesc AssetSuite::Internal::Blob::Describe() const noexcept
{
	return BlobDesc
	{
		sizeof(BlobDesc),
		ByteSize(),
		metadata.format,
		0
	};
}

AssetSuite::Internal::BlobSourceMetadata
AssetSuite::Internal::MakeBlobSourceMetadata(const std::filesystem::path& sourcePath)
{
	BlobSourceMetadata metadata = {};
	metadata.sourcePath = sourcePath;
	metadata.extension = sourcePath.extension();
	metadata.format = ResolveBlobFormat(metadata.extension);
	return metadata;
}

AssetSuite::AssetSuiteBlob_t::AssetSuiteBlob_t(Internal::Blob blob)
	: blob(std::make_unique<Internal::Blob>(std::move(blob)))
{
}

AssetSuite::AssetSuiteBlob_t::~AssetSuiteBlob_t() = default;

AssetSuite::Internal::Blob& AssetSuite::AssetSuiteBlob_t::Blob() noexcept
{
	return *blob;
}

const AssetSuite::Internal::Blob& AssetSuite::AssetSuiteBlob_t::Blob() const noexcept
{
	return *blob;
}
