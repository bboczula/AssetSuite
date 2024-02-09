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
	auto errorCode = assetManager.ImageLoadAndDecode("paint-01-24bpp.png", bmpImageDesc);
	if (errorCode == AssetSuite::ErrorCode::NonExistingFile)
	{
		std::cout << "ERROR: File doesn't exist!" << std::endl;
		return;
	}
	auto getErrorCode = assetManager.ImageGet(AssetSuite::OutputFormat::RGB8, bmpResult);
	assetManager.StoreImageToFile("output.ppm", bmpResult, bmpImageDesc);
}