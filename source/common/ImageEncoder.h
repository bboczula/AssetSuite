#pragma once

#include <vector>

#include "ImageDescriptor.h"

namespace AssetSuite
{
	class ImageEncoder
	{
	public:
		virtual ~ImageEncoder() = default;
		virtual std::vector<BYTE> Encode(const std::vector<BYTE>& buffer, const ImageDescriptor& descriptor) = 0;
	};
}
