#include "PpmEncoder.h"

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

std::vector<BYTE> AssetSuite::PpmEncoder::Encode(const std::vector<BYTE>& buffer, const ImageDescriptor& descriptor)
{
	UINT bpp = 0;
	if (descriptor.format == ImageFormat::RGB8)
	{
		bpp = 3;
	}
	else if (descriptor.format == ImageFormat::RGBA8)
	{
		bpp = 4;
	}
	const auto scanlineWidth = descriptor.width * bpp;

	// Here we should rougly know how much memory we need
	std::vector<BYTE> output;
	output.reserve(AVG_CHARS_FOR_HEADER + (descriptor.height * scanlineWidth * AVG_CHARS_PER_PIXEL_COMP));

	AppendToByteBuffer(output, "P3\n");
	AppendToByteBuffer(output, descriptor.width);
	AppendToByteBuffer(output, " ");
	AppendToByteBuffer(output, descriptor.height);
	AppendToByteBuffer(output, "\n");
	AppendToByteBuffer(output, 255);
	AppendToByteBuffer(output, "\n");

	for (UINT pixelHeight = 0; pixelHeight < descriptor.height; pixelHeight++)
	{
		for (UINT pixelWidth = 0; pixelWidth < (scanlineWidth); pixelWidth++)
		{
			// If we have alpha, we need to skip this
			if (bpp == 4 && ((pixelWidth + 1) % 4) == 0)
			{
				continue;
			}
			AppendToByteBuffer(output, buffer[(pixelHeight * ((scanlineWidth))) + pixelWidth]);
			AppendToByteBuffer(output, " ");
		}
		AppendToByteBuffer(output, "\n");
	}

	return output;
}

void AssetSuite::PpmEncoder::AppendToByteBuffer(std::vector<BYTE>& buffer, const char* data)
{
	auto numOfChars = std::char_traits<char>::length(data);
	std::copy(data, data + numOfChars, std::back_inserter(buffer));
}

void AssetSuite::PpmEncoder::AppendToByteBuffer(std::vector<BYTE>& buffer, const UINT number)
{
	std::array<char, MAX_NUM_OF_CHARS> output;
	auto const result = std::to_chars(output.data(), output.data() + output.size() - 1, number);
	assert(result.ec == std::errc());
	*result.ptr = '\0';
	AppendToByteBuffer(buffer, output.data());
}
