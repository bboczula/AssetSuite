#include "BypassEncoder.h"

#include <iostream>
#include <cassert>
#include <charconv>
#include <array>
#include <sstream>

// The biggest number of chars to fit maximum value of unsigned integer
#define MAX_NUM_OF_CHARS 10

// Used to roughly reserve enough space in the output vector
#define AVG_CHARS_PER_PIXEL_COMP 3

// Roughly number of chars needed to encode the header
#define AVG_CHARS_FOR_HEADER 20

std::vector<BYTE> AssetSuite::BypassEncoder::Encode(const std::vector<BYTE>& buffer, const ImageDescriptor& descriptor)
{
	//std::vector<BYTE> input = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0xFF };
	std::vector<BYTE> output;
	AppendToByteBuffer(output, "std::vector<BYTE> buffer = {\n");
	UINT elementsCount = 0;
	for (auto it = buffer.begin(); it != buffer.end(); it++)
	{
		if (elementsCount == 0)
		{
			AppendToByteBuffer(output, "\t");
		}
		AppendToByteBuffer(output, "0x");
		if (*it < 16)
		{
			AppendToByteBuffer(output, "0");
		}
		AppendToByteBuffer(output, *it);
		if (it + 1 != buffer.end())
		{
			AppendToByteBuffer(output, ",");
		}
		if (++elementsCount == 16)
		{
			AppendToByteBuffer(output, "\n");
			elementsCount = 0;
		}
		else
		{
			AppendToByteBuffer(output, " ");
		}
	}
	AppendToByteBuffer(output, "\n};");
	return output;
}

void AssetSuite::BypassEncoder::AppendToByteBuffer(std::vector<BYTE>& buffer, const char* data)
{
	auto numOfChars = std::char_traits<char>::length(data);
	std::copy(data, data + numOfChars, std::back_inserter(buffer));
}

void AssetSuite::BypassEncoder::AppendToByteBuffer(std::vector<BYTE>& buffer, const UINT number)
{
	std::array<char, MAX_NUM_OF_CHARS> output;
	auto const result = std::to_chars(output.data(), output.data() + output.size() - 1, number, 16);
	assert(result.ec == std::errc());
	*result.ptr = '\0';
	AppendToByteBuffer(buffer, output.data());
}
