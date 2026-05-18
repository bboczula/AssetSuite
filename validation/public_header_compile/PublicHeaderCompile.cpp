#include <cstddef>
#include <cstdint>
#include <type_traits>

// Public SDK headers must not expose legacy, platform, or STL-owning API types.
// Poisoning the tokens before inclusion turns accidental public leakage into a
// compile failure for this external-consumer validation target.
#define BYTE ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(BYTE)
#define FLOAT ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(FLOAT)
#define UINT ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(UINT)
#define vector ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(vector)
#define basic_string ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(basic_string)
#define string ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(string)
#define wstring ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(wstring)
#define u8string ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(u8string)
#define u16string ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(u16string)
#define u32string ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(u32string)
#define filesystem ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(filesystem)
#define path ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(path)
#define Manager ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(Manager)
#define Decoder ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(Decoder)
#define ImageDecoder ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(ImageDecoder)
#define MeshDecoder ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(MeshDecoder)
#define ImageDecoders ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(ImageDecoders)
#define MeshDecoders ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(MeshDecoders)
#define RuntimeContext ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(RuntimeContext)
#define RuntimeState ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(RuntimeState)
#define CodecStorage ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(CodecStorage)
#define CodecRegistry ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(CodecRegistry)
#define AllocatorPolicy ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(AllocatorPolicy)
#define Diagnostics ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(Diagnostics)
#define FileLoader ASSETSUITE_FORBIDDEN_PUBLIC_SYMBOL(FileLoader)

#include <AssetSuite/AssetSuite.h>
#include <AssetSuite/AssetSuiteDescriptors.h>
#include <AssetSuite/AssetSuiteHandles.h>
#include <AssetSuite/AssetSuiteTypes.h>

#if defined(_WINDOWS_) || defined(_INC_WINDOWS) || defined(WINAPI)
#error Public AssetSuite SDK headers must not include Windows.h.
#endif

#undef BYTE
#undef FLOAT
#undef UINT
#undef vector
#undef basic_string
#undef string
#undef wstring
#undef u8string
#undef u16string
#undef u32string
#undef filesystem
#undef path
#undef Manager
#undef Decoder
#undef ImageDecoder
#undef MeshDecoder
#undef ImageDecoders
#undef MeshDecoders
#undef RuntimeContext
#undef RuntimeState
#undef CodecStorage
#undef CodecRegistry
#undef AllocatorPolicy
#undef Diagnostics
#undef FileLoader

static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::Result>, int32_t>);
static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::PixelFormat>, uint32_t>);
static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::AssetFormat>, uint32_t>);
static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::MeshAttributeFlags>, uint32_t>);
static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::LogLevel>, uint32_t>);
static_assert(static_cast<uint32_t>(AssetSuite::LogLevel::Trace) == 0);
static_assert(static_cast<uint32_t>(AssetSuite::LogLevel::Debug) == 1);
static_assert(static_cast<uint32_t>(AssetSuite::LogLevel::Info) == 2);
static_assert(static_cast<uint32_t>(AssetSuite::LogLevel::Warning) == 3);
static_assert(static_cast<uint32_t>(AssetSuite::LogLevel::Error) == 4);
static_assert(static_cast<uint32_t>(AssetSuite::LogLevel::Fatal) == 5);

static_assert(std::is_same_v<
	AssetSuite::LoggingCallback,
	void (*)(AssetSuite::LogLevel, const char*, void*)>);

static_assert(std::is_pointer_v<AssetSuite::ContextHandle>);
static_assert(std::is_pointer_v<AssetSuite::BlobHandle>);
static_assert(std::is_pointer_v<AssetSuite::ImageHandle>);
static_assert(std::is_pointer_v<AssetSuite::MeshHandle>);

static_assert(std::is_standard_layout_v<AssetSuite::ContextDesc>);
static_assert(std::is_standard_layout_v<AssetSuite::BlobDesc>);
static_assert(std::is_standard_layout_v<AssetSuite::ImageDesc>);
static_assert(std::is_standard_layout_v<AssetSuite::MeshDesc>);

static_assert(std::is_trivially_copyable_v<AssetSuite::ContextDesc>);
static_assert(std::is_trivially_copyable_v<AssetSuite::BlobDesc>);
static_assert(std::is_trivially_copyable_v<AssetSuite::ImageDesc>);
static_assert(std::is_trivially_copyable_v<AssetSuite::MeshDesc>);

static_assert(offsetof(AssetSuite::ContextDesc, structSize) == 0);
static_assert(offsetof(AssetSuite::BlobDesc, structSize) == 0);
static_assert(offsetof(AssetSuite::ImageDesc, structSize) == 0);
static_assert(offsetof(AssetSuite::MeshDesc, structSize) == 0);

static_assert(std::is_same_v<
	decltype(&AssetSuite::CreateContext),
	AssetSuite::Result (*)(const AssetSuite::ContextDesc*, AssetSuite::ContextHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::DestroyContext),
	AssetSuite::Result (*)(AssetSuite::ContextHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::SetLoggingCallback),
	AssetSuite::Result (*)(
		AssetSuite::ContextHandle,
		AssetSuite::LoggingCallback,
		AssetSuite::LogLevel,
		void*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::LoadFile),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, const char*, AssetSuite::BlobHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::DecodeImage),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, AssetSuite::BlobHandle, AssetSuite::ImageHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::DecodeImageFromFile),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, const char*, AssetSuite::ImageHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::DecodeMesh),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, AssetSuite::BlobHandle, AssetSuite::MeshHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::DecodeMeshFromFile),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, const char*, AssetSuite::MeshHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::GetImageDesc),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, AssetSuite::ImageHandle, AssetSuite::ImageDesc*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::GetMeshDesc),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, AssetSuite::MeshHandle, AssetSuite::MeshDesc*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::ReleaseImage),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, AssetSuite::ImageHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::ReleaseMesh),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, AssetSuite::MeshHandle*)>);
static_assert(std::is_same_v<
	decltype(&AssetSuite::ReleaseBlob),
	AssetSuite::Result (*)(AssetSuite::ContextHandle, AssetSuite::BlobHandle*)>);

void PublicLoggingCallback(AssetSuite::LogLevel, const char*, void*)
{
}

int main()
{
	AssetSuite::ContextHandle context = nullptr;
	AssetSuite::BlobHandle blob = nullptr;
	AssetSuite::ImageHandle image = nullptr;
	AssetSuite::MeshHandle mesh = nullptr;

	AssetSuite::ContextDesc contextDesc = { sizeof(AssetSuite::ContextDesc), 0 };
	AssetSuite::BlobDesc blobDesc = { sizeof(AssetSuite::BlobDesc), 0, AssetSuite::AssetFormat::Unknown, 0 };
	AssetSuite::ImageDesc imageDesc = { sizeof(AssetSuite::ImageDesc), 0, 0, AssetSuite::PixelFormat::Unknown, 0, 0 };
	AssetSuite::MeshDesc meshDesc = { sizeof(AssetSuite::MeshDesc), 0, 0, 0, 0 };

	const AssetSuite::Result createDefaultResult = AssetSuite::CreateContext(nullptr, &context);
	const AssetSuite::Result destroyDefaultResult = AssetSuite::DestroyContext(&context);
	const AssetSuite::Result createExplicitResult = AssetSuite::CreateContext(&contextDesc, &context);
	const AssetSuite::Result setLoggingResult = AssetSuite::SetLoggingCallback(
		context,
		&PublicLoggingCallback,
		AssetSuite::LogLevel::Warning,
		&contextDesc);
	const AssetSuite::Result unregisterLoggingResult = AssetSuite::SetLoggingCallback(
		context,
		nullptr,
		AssetSuite::LogLevel::Trace,
		nullptr);
	const AssetSuite::Result loadNullPathResult = AssetSuite::LoadFile(context, nullptr, &blob);
	const AssetSuite::Result decodeNullImagePathResult = AssetSuite::DecodeImageFromFile(context, nullptr, &image);
	const AssetSuite::Result decodeNullMeshPathResult = AssetSuite::DecodeMeshFromFile(context, nullptr, &mesh);
	const AssetSuite::Result getNullImageResult = AssetSuite::GetImageDesc(context, image, &imageDesc);
	const AssetSuite::Result getNullMeshResult = AssetSuite::GetMeshDesc(context, mesh, &meshDesc);
	const AssetSuite::Result releaseNullImageResult = AssetSuite::ReleaseImage(context, &image);
	const AssetSuite::Result releaseNullMeshResult = AssetSuite::ReleaseMesh(context, &mesh);
	const AssetSuite::Result releaseNullBlobResult = AssetSuite::ReleaseBlob(context, &blob);
	const AssetSuite::Result destroyExplicitResult = AssetSuite::DestroyContext(&context);

	return context || blob || image || mesh ||
		contextDesc.structSize == 0 ||
		blobDesc.structSize == 0 ||
		imageDesc.structSize == 0 ||
		meshDesc.structSize == 0 ||
		createDefaultResult == AssetSuite::Result::ErrorUnknown ||
		destroyDefaultResult == AssetSuite::Result::ErrorUnknown ||
		createExplicitResult == AssetSuite::Result::ErrorUnknown ||
		setLoggingResult == AssetSuite::Result::ErrorUnknown ||
		unregisterLoggingResult == AssetSuite::Result::ErrorUnknown ||
		loadNullPathResult == AssetSuite::Result::ErrorUnknown ||
		decodeNullImagePathResult == AssetSuite::Result::ErrorUnknown ||
		decodeNullMeshPathResult == AssetSuite::Result::ErrorUnknown ||
		getNullImageResult == AssetSuite::Result::ErrorUnknown ||
		getNullMeshResult == AssetSuite::Result::ErrorUnknown ||
		releaseNullImageResult == AssetSuite::Result::ErrorUnknown ||
		releaseNullMeshResult == AssetSuite::Result::ErrorUnknown ||
		releaseNullBlobResult == AssetSuite::Result::ErrorUnknown ||
		destroyExplicitResult == AssetSuite::Result::ErrorUnknown;
}
