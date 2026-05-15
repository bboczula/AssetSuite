#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <AssetSuite/AssetSuite.h>
#include <AssetSuite/AssetSuiteDescriptors.h>
#include <AssetSuite/AssetSuiteHandles.h>
#include <AssetSuite/AssetSuiteTypes.h>

static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::Result>, int32_t>);
static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::PixelFormat>, uint32_t>);
static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::AssetFormat>, uint32_t>);
static_assert(std::is_same_v<std::underlying_type_t<AssetSuite::MeshAttributeFlags>, uint32_t>);

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

	return context || blob || image || mesh ||
		contextDesc.structSize == 0 ||
		blobDesc.structSize == 0 ||
		imageDesc.structSize == 0 ||
		meshDesc.structSize == 0;
}
