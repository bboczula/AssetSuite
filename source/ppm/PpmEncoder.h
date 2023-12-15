#pragma once

#include <vector>
#include <Windows.h>

#include "../common/ImageEncoder.h"

namespace AssetSuite
{
	class PpmEncoder : public ImageEncoder
	{
	public:
		std::vector<BYTE> Encode(const std::vector<BYTE>& buffer, const ImageDescriptor& descriptor) override;
	private:
		void AppendToByteBuffer(std::vector<BYTE>& buffer, const char* data);
		void AppendToByteBuffer(std::vector<BYTE>& buffer, const UINT number);
	};
}
