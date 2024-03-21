#pragma once

#include "MeshDescriptor.h"

#include <vector>

namespace AssetSuite
{
	class MeshDecoder
	{
	public:
		virtual bool Decode(std::vector<BYTE>& output, BYTE* buffer, MeshDescriptor& descriptor) = 0;
	};
}