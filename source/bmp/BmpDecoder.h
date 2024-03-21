#pragma once

#include <vector>
#include <Windows.h>

#include "../common/ImageDecoder.h"

namespace AssetSuite
{
	class BmpDecoder : public ImageDecoder
	{
	public:
		bool Decode(std::vector<BYTE>& output, BYTE* buffer, ImageDescriptor& descriptor) override;
	private:
		int CalculatePadding(DWORD lineSize);
	};
}
