#include "PngDecoder.h"
#include <bitset>
#include "../zlib/ZLib.h"

#define ENABLE_PRINT 0

bool AssetSuite::PngDecoder::Decode(std::vector<BYTE>& output, BYTE* buffer, ImageDescriptor& descriptor)
{
	// Reset the buffers in case you  are reading multiple files
	scanlines.clear();
	compressedDataBuffer.clear();

	// First 8 bytes is the signature, followed by chunks
	//size_t totalSize = sizeof(BYTE) * 8;
	BYTE pngSignature[8];
	memcpy(&pngSignature, buffer, sizeof(BYTE) * 8);
	buffer += sizeof(BYTE) * 8;

	// Chunk is always the same
	// 4B Length
	// 4B Type
	// *B Data
	// 4B CRC

	// We could have like a chunk buffer, and then based on the type, you could have handlers
	// And you could read untill you find the IEND buffer;

	// It would also be great to have a file buffer, with the data pointer, that you can move
	// And it could actually be a template, couldn't it?
	
	// Here you have to read each chunk until you reach the ened of file
	// Each chunk is 4B length, 4B type, then data and then CRC
	bool exitLoop = false;
	while (!exitLoop)
	{
		ChunkMetadata localChunk;
		exitLoop = ConsumeChunk(localChunk, buffer, descriptor);
		buffer += (sizeof(BYTE) * 4 * 3) + localChunk.Length;
	}

	// Now it is time to decompress the data using the DEFLATE algorithm
	// You have to decompress this to scanlines
#if ENABLE_PRINT
	std::cout << "Compressed data: " << compressedDataBuffer.size() << std::endl;
	std::cout << "Scanlines: " << scanlines.size() << std::endl;

	std::cout << "-----ZLIB-----\n";
#endif

	//for (int i = 0; i < compressedDataBuffer.size(); i++)
	//{
	//	std::cout << "[" << i << "] " << std::bitset<8>(compressedDataBuffer[i]) << std::endl;
	//}
	if (descriptor.format == ImageFormat::Unknown)
	{
		return false;
	}

	ZLib zlib;
	auto result = zlib.Decode(compressedDataBuffer);

	UINT bpp = 0;
	if (descriptor.format == ImageFormat::RGB8)
	{
		bpp = 3;
	}
	else if (descriptor.format == ImageFormat::RGBA8)
	{
		bpp = 4;
	}

	std::vector<BYTE> filtered;
	// Here you could technically interate over scanlines
	for (UINT i = 0; i < descriptor.height; i++)
	{
		// Index of the filtering byte
		auto scanlineWidth = ((descriptor.width * bpp) + 1);
		auto index = (i * scanlineWidth);
		//std::copy(result.begin() + index + 1, result.begin() + index + 1 + (descriptor.width * bpp), std::back_inserter(filtered));
		auto value = result[index];
		switch (value)
		{
		case 0:
			// None
			std::copy(result.begin() + index + 1, result.begin() + index + scanlineWidth, std::back_inserter(filtered));
			break;
		case 1:
			// Sub
			for (UINT i = 1; i < scanlineWidth; i++)
			{
				auto lastIndex = filtered.size();
				BYTE leftValue = i <= bpp ? 0 : filtered[lastIndex - bpp];
				filtered.push_back(result[index + i] + leftValue);
			}
			break;
		case 2:
			// Up
			for (UINT i = 1; i < scanlineWidth; i++)
			{
				auto lastIndex = filtered.size();
				BYTE upValue = index + i < scanlineWidth ? 0 : filtered[lastIndex - scanlineWidth + 1];
				filtered.push_back(result[index + i] + upValue);
			}
			break;
		case 3:
			// Average
			for (UINT i = 1; i < scanlineWidth; i++)
			{
				auto lastIndex = filtered.size();
				BYTE leftValue = i <= bpp ? 0 : filtered[lastIndex - bpp];
				BYTE upValue = index + i < scanlineWidth ? 0 : filtered[lastIndex - scanlineWidth + 1];
				filtered.push_back(result[index + i] + (leftValue + upValue) / 2);
			}
			break;
		case 4:
			// Peath
			for (UINT i = 1; i < scanlineWidth; i++)
			{
				const BOOL isFirstLine = index + i < scanlineWidth;
				const BOOL isFirstPixel = i <= bpp;
				const auto lastIndex = filtered.size();
				BYTE leftValue = isFirstPixel ? 0 : filtered[lastIndex - bpp];
				BYTE upValue = isFirstLine ? 0 : filtered[lastIndex - scanlineWidth + 1];
				BYTE upLeftValue = isFirstLine || isFirstPixel ? 0 : filtered[lastIndex - scanlineWidth + 1 - bpp];
				filtered.push_back(result[index + i] + PaethPreditor(leftValue, upValue, upLeftValue));
			}
			break;
		default:
			std::copy(result.begin() + index + 1, result.begin() + index + ((descriptor.width * bpp) + 1), std::back_inserter(filtered));
			break;
		}
	}
	output = filtered;
	return true;
}

BYTE AssetSuite::PngDecoder::Convert1Byte(const BYTE* buffer)
{
	return buffer[0];
}

unsigned long AssetSuite::PngDecoder::Convert4Bytes(const BYTE* buffer)
{
	return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

unsigned long AssetSuite::PngDecoder::ConvertBits(const BYTE* buffer, UINT& byteIndex, UINT& bitIndex, UINT numOfBits)
{
	if (numOfBits + bitIndex <= 8)
	{
		BYTE currentByte = Convert1Byte(buffer + (sizeof(BYTE) * byteIndex));
		BYTE mask = (0x1 << numOfBits) - 1;
		mask = mask << bitIndex;
		//std::cout << "Current byte: " << std::bitset<8>(currentByte) << std::endl;
		//std::cout << "Mask: " << std::bitset<8>(mask) << std::endl;

		unsigned long returnValue = (currentByte & mask) >> bitIndex;

		// You need to figure out the numbers here
		// Now that you consume some bits, you need to set bitIndex
		bitIndex += numOfBits;

		// Here we need to figure out if we go to the next byte
		if (bitIndex == 8)
		{
			bitIndex = 0;
			byteIndex++;
		}

		return returnValue;
	}
	else if (numOfBits + bitIndex <= 16)
	{
		//std::cout << "ERROR: We need at least two bytes for this\n";
		// I need two bytes, but do I need to flip them? How to handle this?

		BYTE b1 = Convert1Byte(buffer + (sizeof(BYTE) * byteIndex));
		BYTE b2 = Convert1Byte(buffer + (sizeof(BYTE) * byteIndex) + 1);
		//std::cout << std::bitset<8>(b2) << std::bitset<8>(b1) << std::endl;
		//USHORT currentTwoBytes = Convert2Bytes(buffer + (sizeof(BYTE) * byteIndex));
		//std::cout << std::bitset<16>(currentTwoBytes) << std::endl;
		// I don't think I need to flip this, though it would be great to understand why
		// Yeah, this sounds about right
		USHORT noFlip = (b2 << 8) | b1;
		//std::cout << std::bitset<16>(noFlip) << std::endl;

		USHORT mask = (0x1 << numOfBits) - 1;
		mask = mask << bitIndex;
		//std::cout << "Current byte: " << std::bitset<16>(noFlip) << std::endl;
		//std::cout << "Mask: " << std::bitset<16>(mask) << std::endl;
		unsigned long returnValue = (noFlip & mask) >> bitIndex;

		// Now figure out the numbers
		bitIndex += numOfBits;
		if (bitIndex >= 8)
		{
			bitIndex -= 8;
			byteIndex++;
		}

		return returnValue;
	}
	else
	{
		std::cout << "ERROR: We're not handling this stuff yet!\n";
	}
	return 0;
}

unsigned long AssetSuite::PngDecoder::Convert2Bytes(const BYTE* buffer)
{
	return (buffer[0] << 8) | buffer[1];
}

bool AssetSuite::PngDecoder::ConsumeChunk(ChunkMetadata& chunk, BYTE* dataPointer, ImageDescriptor& descriptor)
{
	// Read chunk metadata
	chunk.Length = Convert4Bytes(dataPointer);
	chunk.Type = BytesToChunkType(dataPointer + (sizeof(BYTE) * 4));
	chunk.Data = dataPointer + (2 * (sizeof(BYTE) * 4));
	chunk.CRC = Convert4Bytes(dataPointer + (2 * (sizeof(BYTE) * 4)) + chunk.Length);

	// Error checking and CRC

	// Process chunk based on type
	switch (chunk.Type)
	{
	case(ChunkType::IHDR):
#if ENABLE_PRINT
		std::cout << "IHDR\n";
#endif
		Process_IHDR(chunk.Data, descriptor);
		return false;
	case(ChunkType::IEND):
#if ENABLE_PRINT
		std::cout << "IEND\n";
#endif
		return true;
	case(ChunkType::sRGB):
#if ENABLE_PRINT
		std::cout << "sRGB\n";
#endif
		Process_sRGB(chunk.Data);
		return false;
	case(ChunkType::gAMA):
#if ENABLE_PRINT
		std::cout << "gAMA\n";
#endif
		Process_gAMA(chunk.Data);
		return false;
	case(ChunkType::pHYs):
#if ENABLE_PRINT
		std::cout << "pHYs\n";
#endif
		Process_pHYs(chunk.Data);
		return false;
	case(ChunkType::IDAT):
#if ENABLE_PRINT
		std::cout << "IDAT\n";
#endif
		Process_IDAT(chunk.Data, chunk.Length);
		return false;
	default:
		std::cout << "ERROR: Unknown chunk type!\n";
		return false;
	}

	// Move the data pointer forward the proper amount
}

AssetSuite::ChunkType AssetSuite::PngDecoder::BytesToChunkType(const BYTE* buffer)
{
	BYTE pngTypeNull[5];
	memcpy(&pngTypeNull, buffer, sizeof(BYTE) * 4);
	pngTypeNull[4] = '\0';
	if (strcmp((char*)pngTypeNull, "IHDR\0") == 0)
	{
		return ChunkType::IHDR;
	}
	else if (strcmp((char*)pngTypeNull, "sRGB\0") == 0)
	{
		return ChunkType::sRGB;
	}
	else if (strcmp((char*)pngTypeNull, "gAMA\0") == 0)
	{
		return ChunkType::gAMA;
	}
	else if (strcmp((char*)pngTypeNull, "pHYs\0") == 0)
	{
		return ChunkType::pHYs;
	}
	else if (strcmp((char*)pngTypeNull, "IDAT\0") == 0)
	{
		return ChunkType::IDAT;
	}
	else if (strcmp((char*)pngTypeNull, "IEND\0") == 0)
	{
		return ChunkType::IEND;
	}
	else
	{
		return ChunkType::Undefined;
	}
}

void AssetSuite::PngDecoder::Process_sRGB(BYTE* chunkData)
{
	sRGB chunk;
	chunk.renderingIntent = Convert1Byte(chunkData);
#if ENABLE_PRINT
	std::cout << "\tsRGB.renderingIntent = " << (UINT)chunk.renderingIntent << std::endl;
#endif
}

void AssetSuite::PngDecoder::Process_gAMA(BYTE* chunkData)
{
	gAMA chunk;
	chunk.gamma = Convert1Byte(chunkData);
#if ENABLE_PRINT
	std::cout << "\tgAMA.gamma = " << (UINT)chunk.gamma << std::endl;
#endif
}

void AssetSuite::PngDecoder::Process_pHYs(BYTE* chunkData)
{
	pHYs chunk;
	chunk.pixelsPerUnitX = Convert4Bytes(chunkData);
	chunk.pixelsPerUnitY = Convert4Bytes(chunkData);
	chunk.unitSpecifier = Convert1Byte(chunkData);
#if ENABLE_PRINT
	std::cout << "\tpHYs.pixelsPerUnitX = " << chunk.pixelsPerUnitX << std::endl;
	std::cout << "\tpHYs.pixelsPerUnitY = " << chunk.pixelsPerUnitY << std::endl;
	std::cout << "\tpHYs.unitSpecifier = " << (UINT)chunk.unitSpecifier << std::endl;
#endif
}

void AssetSuite::PngDecoder::Process_IDAT(BYTE* chunkData, UINT chunkDataLength)
{
	compressedDataBuffer.insert(compressedDataBuffer.end(), chunkData, chunkData + chunkDataLength);
#if ENABLE_PRINT
	std::cout << "\tcontains " << chunkDataLength << " bytes" << std::endl;
#endif
}

BYTE AssetSuite::PngDecoder::PaethPreditor(BYTE left, BYTE up, BYTE upperLeft)
{
	auto p = left + up - upperLeft;
	auto pa = std::abs(p - left);
	auto pb = std::abs(p - up);
	auto pc = std::abs(p - upperLeft);
	if (pa <= pb && pa <= pc)
	{
		return left;
	}
	else if (pb <= pc)
	{
		return up;
	}
	else
	{
		return upperLeft;
	}
}

void AssetSuite::PngDecoder::Process_IHDR(BYTE* chunkData, ImageDescriptor& descriptor)
{
	IHDR chunk;
	//memcpy(&chunk, buffer + totalSize, sizeof(IHDR));
	chunk.width = Convert4Bytes(chunkData);
	//totalSize += sizeof(BYTE) * 4;
	chunk.height = Convert4Bytes(chunkData + (sizeof(BYTE) * 4));
	//totalSize += sizeof(BYTE) * 4;
	chunk.bitDepth = Convert1Byte(chunkData + (sizeof(BYTE) * 8));
	//totalSize += sizeof(BYTE);
	chunk.colorType = Convert1Byte(chunkData + (sizeof(BYTE) * 9));
	//totalSize += sizeof(BYTE);
	chunk.compressionMethod = Convert1Byte(chunkData + (sizeof(BYTE) * 10));
	//totalSize += sizeof(BYTE);
	chunk.filterMethod = Convert1Byte(chunkData + (sizeof(BYTE) * 11));
	//totalSize += sizeof(BYTE);
	chunk.interlanceMethod = Convert1Byte(chunkData + (sizeof(BYTE) * 12));
	//totalSize += sizeof(BYTE);

	descriptor.width = chunk.width;
	descriptor.height = chunk.height;
	if (chunk.colorType == 2)
	{
		descriptor.format = ImageFormat::RGB8;
	}
	else if (chunk.colorType == 6)
	{
		descriptor.format = ImageFormat::RGBA8;
	}
	else
	{
		descriptor.format = ImageFormat::Unknown;
	}

	scanlines.resize(chunk.width * chunk.height * chunk.bitDepth);
#if ENABLE_PRINT
	std::cout << "\tIHDR.width = " << chunk.width << std::endl;
	std::cout << "\tIHDR.height = " << chunk.height << std::endl;
	std::cout << "\tIHDR.bitDepth = " << (UINT)chunk.bitDepth << std::endl;
	std::cout << "\tIHDR.colorType = " << (UINT)chunk.colorType << std::endl;
	std::cout << "\tIHDR.compressionMethod = " << (UINT)chunk.compressionMethod << std::endl;
	std::cout << "\tIHDR.filterMethod = " << (UINT)chunk.filterMethod << std::endl;
	std::cout << "\tIHDR.interlanceMethod = " << (UINT)chunk.interlanceMethod << std::endl;
#endif
}
