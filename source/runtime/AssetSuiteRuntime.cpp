#include "AssetSuiteRuntime.h"

#include <new>
#include <utility>
#include <vector>

namespace
{
	AssetSuite::PixelFormat MapImagePixelFormat(AssetSuite::ImageFormat format) noexcept
	{
		switch (format)
		{
		case AssetSuite::ImageFormat::RGB8:
			return AssetSuite::PixelFormat::RGB8;
		case AssetSuite::ImageFormat::RGBA8:
			return AssetSuite::PixelFormat::RGBA8;
		default:
			return AssetSuite::PixelFormat::Unknown;
		}
	}

	bool HasMinimumImageDecodeBytes(AssetSuite::ImageDecoders decoder, const AssetSuite::Internal::Blob& blob) noexcept
	{
		switch (decoder)
		{
		case AssetSuite::ImageDecoders::BMP:
			return blob.ByteSize() >= 54;
		case AssetSuite::ImageDecoders::PNG:
			return blob.ByteSize() >= 8;
		default:
			return blob.ByteSize() > 0;
		}
	}

	bool HasMinimumMeshDecodeBytes(AssetSuite::MeshDecoders decoder, const AssetSuite::Internal::Blob& blob) noexcept
	{
		switch (decoder)
		{
		case AssetSuite::MeshDecoders::WAVEFRONT:
			return blob.ByteSize() > 0;
		default:
			return blob.ByteSize() > 0;
		}
	}

	AssetSuite::Result MapSelectedDecoderFailure() noexcept
	{
		// Current decoder interfaces return only bool, so a selected decoder
		// rejecting bytes is the best available malformed-input signal.
		return AssetSuite::Result::ErrorMalformedData;
	}
}

AssetSuite::Internal::RuntimeContext::RuntimeContext(const ContextDesc& desc)
	: desc(desc)
	, state()
	, manager(state)
	, logging()
{
}

AssetSuite::Internal::RuntimeContext::~RuntimeContext() = default;

const AssetSuite::ContextDesc& AssetSuite::Internal::RuntimeContext::Descriptor() const noexcept
{
	return desc;
}

AssetSuite::Manager& AssetSuite::Internal::RuntimeContext::LegacyManager() noexcept
{
	return manager;
}

const AssetSuite::Manager& AssetSuite::Internal::RuntimeContext::LegacyManager() const noexcept
{
	return manager;
}

AssetSuite::Internal::RuntimeState::AllocatorPolicy&
AssetSuite::Internal::RuntimeContext::AllocatorPolicy() noexcept
{
	return state.allocatorPolicy;
}

AssetSuite::Internal::RuntimeState::Diagnostics&
AssetSuite::Internal::RuntimeContext::Diagnostics() noexcept
{
	return state.diagnostics;
}

const AssetSuite::Internal::RuntimeState::Diagnostics&
AssetSuite::Internal::RuntimeContext::Diagnostics() const noexcept
{
	return state.diagnostics;
}

AssetSuite::Internal::RuntimeState::FileLoader&
AssetSuite::Internal::RuntimeContext::FileLoader() noexcept
{
	return state.fileLoader;
}

AssetSuite::Internal::RuntimeState::BlobStorage&
AssetSuite::Internal::RuntimeContext::BlobStorage() noexcept
{
	return state.blobStorage;
}

const AssetSuite::Internal::RuntimeState::BlobStorage&
AssetSuite::Internal::RuntimeContext::BlobStorage() const noexcept
{
	return state.blobStorage;
}

AssetSuite::Internal::RuntimeState::ImageStorage&
AssetSuite::Internal::RuntimeContext::ImageStorage() noexcept
{
	return state.imageStorage;
}

const AssetSuite::Internal::RuntimeState::ImageStorage&
AssetSuite::Internal::RuntimeContext::ImageStorage() const noexcept
{
	return state.imageStorage;
}

AssetSuite::Internal::RuntimeState::MeshStorage&
AssetSuite::Internal::RuntimeContext::MeshStorage() noexcept
{
	return state.meshStorage;
}

const AssetSuite::Internal::RuntimeState::MeshStorage&
AssetSuite::Internal::RuntimeContext::MeshStorage() const noexcept
{
	return state.meshStorage;
}

AssetSuite::Internal::RuntimeState::CodecRegistry&
AssetSuite::Internal::RuntimeContext::CodecRegistry() noexcept
{
	return state.codecRegistry;
}

const AssetSuite::Internal::RuntimeState::CodecRegistry&
AssetSuite::Internal::RuntimeContext::CodecRegistry() const noexcept
{
	return state.codecRegistry;
}

AssetSuite::Result AssetSuite::Internal::RuntimeContext::DecodeImageBlob(
	const Blob& blob,
	ImageHandle* outImage)
{
	const ImageDecoders decoder = CodecRegistry().ProbeImageDecoder(
		blob.SourceMetadata().extension,
		blob.Data(),
		static_cast<size_t>(blob.ByteSize()));
	if (decoder == ImageDecoders::Auto)
	{
		Diagnostics().Add(ErrorCode::FileTypeNotSupported, "Image blob format is not supported.");
		return Result::ErrorUnsupportedFormat;
	}

	if (!HasMinimumImageDecodeBytes(decoder, blob))
	{
		Diagnostics().Add(ErrorCode::Undefined, "Image blob is too small for the selected decoder.");
		return Result::ErrorMalformedData;
	}

	ImageDecoder* imageDecoder = CodecRegistry().FindImageDecoder(decoder);
	if (!imageDecoder)
	{
		Diagnostics().Add(ErrorCode::FileTypeNotSupported, "Image decoder is not registered.");
		return Result::ErrorUnsupportedFormat;
	}

	std::vector<BYTE> decodedBytes;
	ImageDescriptor descriptor = {};
	if (!imageDecoder->Decode(decodedBytes, const_cast<BYTE*>(reinterpret_cast<const BYTE*>(blob.Data())), descriptor))
	{
		Diagnostics().Add(ErrorCode::Undefined, "Image decoder rejected malformed data.");
		return MapSelectedDecoderFailure();
	}

	ImageDesc publicDesc = {
		sizeof(ImageDesc),
		descriptor.width,
		descriptor.height,
		MapImagePixelFormat(descriptor.format),
		1,
		1
	};

	try
	{
		*outImage = ImageStorage().Create(publicDesc, std::move(decodedBytes));
	}
	catch (const std::bad_alloc&)
	{
		return Result::ErrorOutOfMemory;
	}
	catch (...)
	{
		return Result::ErrorUnknown;
	}

	return Result::Success;
}

AssetSuite::Result AssetSuite::Internal::RuntimeContext::DecodeMeshBlob(
	const Blob& blob,
	MeshHandle* outMesh)
{
	const MeshDecoders decoder = CodecRegistry().ProbeMeshDecoder(
		blob.SourceMetadata().extension,
		blob.Data(),
		static_cast<size_t>(blob.ByteSize()));
	if (decoder == MeshDecoders::Auto)
	{
		Diagnostics().Add(ErrorCode::FileTypeNotSupported, "Mesh blob format is not supported.");
		return Result::ErrorUnsupportedFormat;
	}

	if (!HasMinimumMeshDecodeBytes(decoder, blob))
	{
		Diagnostics().Add(ErrorCode::Undefined, "Mesh blob is too small for the selected decoder.");
		return Result::ErrorMalformedData;
	}

	MeshDecoder* meshDecoder = CodecRegistry().FindMeshDecoder(decoder);
	if (!meshDecoder)
	{
		Diagnostics().Add(ErrorCode::FileTypeNotSupported, "Mesh decoder is not registered.");
		return Result::ErrorUnsupportedFormat;
	}

	std::vector<BYTE> decodeBuffer(blob.Data(), blob.Data() + static_cast<size_t>(blob.ByteSize()));
	decodeBuffer.push_back('\0');

	std::vector<BYTE> decodedBytes;
	MeshDescriptor descriptor = {};
	if (!meshDecoder->Decode(decodedBytes, decodeBuffer.data(), descriptor))
	{
		Diagnostics().Add(ErrorCode::Undefined, "Mesh decoder rejected malformed data.");
		return MapSelectedDecoderFailure();
	}

	MeshDesc publicDesc = {
		sizeof(MeshDesc),
		descriptor.numOfVertices,
		descriptor.numOfIndices,
		0,
		0
	};

	try
	{
		*outMesh = MeshStorage().Create(publicDesc, std::move(decodedBytes));
	}
	catch (const std::bad_alloc&)
	{
		return Result::ErrorOutOfMemory;
	}
	catch (...)
	{
		return Result::ErrorUnknown;
	}

	return Result::Success;
}

void AssetSuite::Internal::RuntimeContext::SetLoggingCallback(
	LoggingCallback callback,
	LogLevel minimumLevel,
	void* userData) noexcept
{
	logging.callback = callback;
	logging.minimumLevel = minimumLevel;
	logging.userData = userData;
}

void AssetSuite::Internal::RuntimeContext::DispatchLogEvent(LogLevel level, const char* message) const
{
	if (!logging.callback || !message)
	{
		return;
	}

	if (static_cast<uint32_t>(level) < static_cast<uint32_t>(logging.minimumLevel))
	{
		return;
	}

	logging.callback(level, message, logging.userData);
}

void AssetSuite::Internal::RuntimeContext::EmitDiagnostic(
	ErrorCode code,
	LogLevel level,
	const char* message)
{
	Diagnostics().Add(code, message);
	DispatchLogEvent(level, message);
}
