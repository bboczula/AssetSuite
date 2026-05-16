#pragma once

#include <vector>

#include "ImageDescriptor.h"

namespace AssetSuite
{
	class ImageEncoder
	{
		virtual std::vector<BYTE> Encode(const std::vector<BYTE>& buffer, const ImageDescriptor& descriptor) = 0;
	};
}