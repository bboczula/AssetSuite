#pragma once

#include "ImageDescriptor.h"

namespace AssetSuite
{
	enum DecoderError
	{
		NoDecoderError,
		ColorTypeNotSupported
	};

	class ImageDecoder
	{
	public:
		virtual DecoderError Decode(std::vector<BYTE>& output, BYTE* buffer, ImageDescriptor& descriptor) = 0;
	};
}