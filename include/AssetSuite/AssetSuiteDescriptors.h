#pragma once

#include <cstdint>

#include "AssetSuiteTypes.h"

namespace AssetSuite
{
	struct ContextDesc
	{
		// Must be exactly sizeof(ContextDesc) when a descriptor is supplied.
		uint32_t structSize;
		// Reserved for future use. Must be zero.
		uint32_t flags;
	};

	struct BlobDesc
	{
		uint32_t structSize;
		uint64_t byteSize;
		AssetFormat format;
		uint32_t flags;
	};

	struct ImageDesc
	{
		uint32_t structSize;
		uint32_t width;
		uint32_t height;
		PixelFormat format;
		uint32_t mipCount;
		uint32_t arraySize;
	};

	struct MeshDesc
	{
		uint32_t structSize;
		uint32_t vertexCount;
		uint32_t indexCount;
		uint32_t submeshCount;
		uint32_t attributeFlags;
	};
}
