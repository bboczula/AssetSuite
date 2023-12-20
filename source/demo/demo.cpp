#include <iostream>
#include <vector>
#include <ctime>    // For random number generation
#include <cstdlib>  // For random number generation
#include "../common/AssetSuite.h"

void main()
{
	AssetSuite::Manager assetManager;
	std::cout << "Hello, AssetSuite!" << std::endl;

	AssetSuite::ImageDescriptor bmpImageDesc = {};
	std::vector<BYTE> bmpResult;
	auto errorCode = assetManager.LoadImageFromFile("paint-01-24bpp.bmp", bmpImageDesc, bmpResult);
	assetManager.StoreImageToFile("output.ppm", bmpResult, bmpImageDesc);
}