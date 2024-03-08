#pragma once

#include "MeshDescriptor.h"

#include <vector>

namespace AssetSuite
{
	enum MeshDecoderError
	{
		MeshNoDecoderError,
		MeshColorTypeNotSupported
	};

	class MeshDecoder
	{
	public:
		virtual MeshDecoderError Decode(std::vector<BYTE>& output, BYTE* buffer, MeshDescriptor& descriptor) = 0;
	};
}