#pragma once

#include <vector>
#include <Windows.h>
#include <iostream>

#include "../common/ImageDecoder.h"

namespace AssetSuite
{
	enum class ChunkType
	{
		IHDR,
		sRGB,
		gAMA,
		pHYs,
		IDAT,
		IEND,
		Undefined
	};

	struct ChunkMetadata
	{
		UINT Length;
		ChunkType Type;
		BYTE* Data;
		UINT CRC;
		BOOL IsCritical;
		BOOL IsPrivate;
		BOOL IsSafeToCopy;
	};

	struct IHDR
	{
		UINT width;
		UINT height;
		BYTE bitDepth;
		BYTE colorType;
		BYTE compressionMethod;
		BYTE filterMethod;
		BYTE interlanceMethod;
	};

	enum ColorType
	{
		GrayScale = 0,
		TrueColor = 2,
		IndexedColor = 3,
		GrayScaleWithAlpha = 4,
		TrueColorAlpha = 6
	};

	enum Error
	{
		NoError,
		IncorrectPngSignature
	};

	struct sRGB
	{
		BYTE renderingIntent;
	};

	struct gAMA
	{
		UINT gamma;
	};

	struct pHYs
	{
		UINT pixelsPerUnitX;
		UINT pixelsPerUnitY;
		BYTE unitSpecifier;
	};

	// We need some kind of BitStream or bit stream buffer
	// We already have a stream of bytes in form of the BYTE vector
	// Now we kind of want to traverse it and then be able to convert a varying number of bits into UINT
	// UINT GetBits(BYTE*, UINT numOfBits);
	// CM = GetBits(compressedData.data(), 1);
	// I think it makes most sense if you had read the bits and then move the BitPointer forward
	// You would also need a BytePointer
	// So you would start with BYTE 0 and BIT 0, and then ask to read say 4 bits
	// - you have 0 BYTES read, so you don't really have the material
	// - you should know that in order to give back 4 bits, you need at least 1 byte
	// - you should read the enough bytes to a cache to be able to return the bits
	// - then, move forward the byte and bit pointer (yes, kind of serialized)
	// - finally, return the given amount of bits as an UINT

	class PngDecoder : public ImageDecoder
	{
	public:
		bool Decode(std::vector<BYTE>& output, BYTE* buffer, ImageDescriptor& descriptor) override;
	private:
		std::vector<BYTE> compressedDataBuffer;
		std::vector<BYTE> scanlines;
		BYTE Convert1Byte(const BYTE* buffer);
		unsigned long Convert2Bytes(const BYTE* buffer);
		unsigned long Convert4Bytes(const BYTE* buffer);
		unsigned long ConvertBits(const BYTE* buffer, UINT& byteIndex, UINT& bitIndex, UINT numOfBits);
		bool ConsumeChunk(ChunkMetadata& chunk, BYTE* dataPointer, ImageDescriptor& descriptor);
		ChunkType BytesToChunkType(const BYTE* buffer);
		void Process_IHDR(BYTE* chunkData, ImageDescriptor& descriptor);
		void Process_sRGB(BYTE* chunkData);
		void Process_gAMA(BYTE* chunkData);
		void Process_pHYs(BYTE* chunkData);
		void Process_IDAT(BYTE* chunkData, UINT chunkDataLength);
		BYTE PaethPreditor(BYTE left, BYTE up, BYTE upperLeft);
	};
}