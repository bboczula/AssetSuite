#pragma once

#include "ImageDescriptor.h"

namespace AssetSuite
{
	class ImageDecoder
	{
	public:
		virtual bool Decode(std::vector<BYTE>& output, BYTE* buffer, ImageDescriptor& descriptor) = 0;
	};
}