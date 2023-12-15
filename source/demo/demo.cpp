#include <iostream>
#include <vector>
#include <ctime>    // For random number generation
#include <cstdlib>  // For random number generation
#include "../common/AssetSuite.h"

void main()
{
	AssetSuite::Manager assetManager;
	std::cout << "Hello, AssetSuite!" << std::endl;

	AssetSuite::ImageDescriptor customImageDesc = {};
	customImageDesc.width = 512;
	customImageDesc.height = 512;
	customImageDesc.format = AssetSuite::ImageFormat::RGB8;

	// Define the colors for the gradient
	const int startR = 7, startG = 11, startB =52; // Red
	const int endR = 72, endG = 52, endB = 255;     // Blue

	// Seed the random number generator with the current time
	std::srand(static_cast<unsigned>(std::time(nullptr)));
	const int numStars = 500;

	std::vector<BYTE> customBuffer;
	for (int i = 0; i < customImageDesc.height; i++)
	{
		for (int j = 0; j < customImageDesc.width; j++)
		{
			// Randomly position stars
			if (std::rand() % numStars == 0)
			{
				customBuffer.push_back(255);
				customBuffer.push_back(255);
				customBuffer.push_back(255);
			}
			else
			{
				customBuffer.push_back(startR + (i * (endR - startR)) / customImageDesc.width);
				customBuffer.push_back(startG + (i * (endG - startG)) / customImageDesc.width);
				customBuffer.push_back(startB + (i * (endB - startB)) / customImageDesc.width);
			}
		}
	}
	assetManager.StoreImageToFile("output.ppm", customBuffer, customImageDesc);

	AssetSuite::ImageDescriptor bmpImageDesc = {};
	bmpImageDesc.width = 64;
	bmpImageDesc.height = 64;
	bmpImageDesc.format = AssetSuite::ImageFormat::RGB8;
	std::vector<BYTE> bmpResult;
	auto errorCode = assetManager.LoadImageFromFile("test.bmp", bmpImageDesc, bmpResult);
	assetManager.StoreImageToFile("bmp_output.ppm", bmpResult, bmpImageDesc);

	AssetSuite::ImageDescriptor pngImageDesc = {};
	std::vector<BYTE> pngResult;
	auto pngErrorCode = assetManager.LoadImageFromFile("f99n0g04.png", pngImageDesc, pngResult);
	assetManager.StoreImageToFile("png_output.ppm", pngResult, pngImageDesc);
}