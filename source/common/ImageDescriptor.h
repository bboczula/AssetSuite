#pragma once

#include <Windows.h>

namespace AssetSuite
{
	enum ImageFormat
	{
		Unknown,
		RGB8,
		RGBA8
	};

	struct ImageDescriptor
	{
		UINT width;
		UINT height;
		ImageFormat format;
	};
}