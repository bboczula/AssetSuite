#include "BitStream.h"

#include <assert.h>

void BitStream::SetData(const BYTE* input, size_t size)
{
	data.insert(data.end(), input, input + (sizeof(BYTE) * size));
}

BYTE BitStream::GetByte(UINT index)
{
	return data[index];
}

BYTE BitStream::GetByte()
{
	return data[byteOffset++];
}

size_t BitStream::Size()
{
	return data.size();
}

UINT BitStream::GetBits(size_t numOfBits)
{
	// Is it possible to return values that won't fit BYTE?
	// There has to be some maksimum number that we would allow
	UINT result = {};
	if (numOfBits + bitOffset <= 8)
	{
		result = ConvertBits<BYTE>(data[byteOffset], numOfBits, bitOffset);
	}
	else if (numOfBits + bitOffset <= 16)
	{
		USHORT combinedBytes = (data[byteOffset + 1] << 8) | data[byteOffset];
		result = ConvertBits<USHORT>(combinedBytes, numOfBits, bitOffset);
	}
	else if (numOfBits + bitOffset <= 32)
	{
		UINT combinedBytes = (data[byteOffset + 2] << 16) | (data[byteOffset + 1] << 8) | data[byteOffset];
		result = ConvertBits<UINT>(combinedBytes, numOfBits, bitOffset);
	}
	else
	{
		assert(false);
	}

	bitOffset += static_cast<UINT>(numOfBits);
	if (bitOffset >= 8 && bitOffset < 16)
	{
		bitOffset -= 8;
		byteOffset++;
	}
	else if (bitOffset >= 16 && bitOffset < 32)
	{
		bitOffset -= 16;
		byteOffset += 2;
	}

	return result;
}
