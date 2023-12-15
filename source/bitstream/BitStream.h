#pragma once

#include <Windows.h>
#include <vector>

class BitStream
{
public:
	void SetData(const BYTE* input, size_t size);
	BYTE GetByte(UINT index);
	BYTE GetByte();
	UINT GetBits(size_t numOfBits);
	size_t Size();
	template<typename T>
	T ConvertBits(T value, size_t numOfBits, UINT bitOffset)
	{
		T mask = ((0x1 << numOfBits) - 0x1) << bitOffset;
		return ((value & mask) >> bitOffset);
	}
private:
	std::vector<BYTE> data;
	UINT byteOffset = 0;
	UINT bitOffset = 0;
};
