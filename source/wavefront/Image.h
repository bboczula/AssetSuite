#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <Windows.h>
#include <algorithm>

class Image
{
public:
	Image();
	~Image();
	void loadFromFileBmp(std::string fileName);
	void loadFromFilePng(std::string fileName);
	BYTE* getPixelData();
	unsigned char* getPixelDataPng();
	DWORD getWidth();
	DWORD getHeight();
	BOOL hasAlphaChannel();
	enum Format
	{
		UNKNOWN, B8G8R8, R8G8B8A8
	};
	Format getFormat();
private:
	int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);
	int decodeBMP();
	void readFileToMemory();
	int calculatePadding(DWORD lineSize);
	std::string fileName;
	Format format;
	WORD bitsPerPixel;
	DWORD pixelDataOffset;
	UINT numOfColorsInPalette;
	DWORD width;
	DWORD height;
	BOOL hasAlpha;
	std::vector<unsigned char> buffer;
	std::vector<unsigned char> image;
};