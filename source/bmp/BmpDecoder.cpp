#include "BmpDecoder.h"

AssetSuite::DecoderError AssetSuite::BmpDecoder::Decode(std::vector<BYTE>& output, BYTE* buffer, ImageDescriptor& descriptor)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	// The buffer could be null, if you didn't read anything there
	memcpy(&fileHeader, buffer, sizeof(BITMAPFILEHEADER));
	memcpy(&infoHeader, buffer + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));

	WORD pixelDataOffset = fileHeader.bfOffBits;
	DWORD bitsPerPixel = infoHeader.biBitCount;
	DWORD height = infoHeader.biHeight;
	DWORD width = infoHeader.biWidth;

	// The bits representing the bitmap pixels are packed in rows, also known
	// as strides or scan lines. The size of each row is rounded up to a multiple
	// of 4 bytes (a 32-bit DWORD) by padding. For images with height above 1,
	// multiple padded rows are stored consecutively, forming a PixelArray

	// Calculate proper byte line width
	// ERROR: this 8 is magic number, works only for 24-bit
	const int MAGIC = bitsPerPixel / 8;
	const DWORD byteLineSize = width * MAGIC;
	auto padding = CalculatePadding(byteLineSize);
	const DWORD fileLineSize = byteLineSize + padding;

	// The tricky part is, the image is upside down, it has optional padding for every line, and each pixel is in BGR, not RGB
	//std::vector<BYTE> output;
	output.resize(height * byteLineSize);
	for (int i = 0; i < height; i++)
	{
		memcpy(output.data() + (height - 1 - i) * byteLineSize, buffer + pixelDataOffset + i * fileLineSize, byteLineSize);
	}

	// BMPs are usually stored "bottom-up"

	// BMP seems to be in BGR format, not RGB, I need to revert it
	descriptor.width = width;
	descriptor.height = height;
	descriptor.format = bitsPerPixel == 24 ? ImageFormat::RGB8 : ImageFormat::Unknown;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width * 3; j += 3)
		{
			auto temp = output[(i * byteLineSize) + j];
			output[(i * byteLineSize) + j] = output[(i * byteLineSize) + j + 2];
			output[(i * byteLineSize) + j + 2] = temp;
		}
	}

	return DecoderError::NoDecoderError;
}

int AssetSuite::BmpDecoder::CalculatePadding(DWORD lineSize)
{
	auto padding = (lineSize % 4);
	return(padding ? 4 - padding : 0);
}
