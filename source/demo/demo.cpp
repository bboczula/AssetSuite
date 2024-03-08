#include <iostream>
#include <vector>
#include <ctime>    // For random number generation
#include <cstdlib>  // For random number generation
#include "../common/AssetSuite.h"

#define DUMP_BUFFERS 0

void main()
{
	AssetSuite::Manager assetManager;
	std::cout << "Hello, AssetSuite!" << std::endl;

	AssetSuite::ImageDescriptor bmpImageDesc = {};
	std::vector<BYTE> bmpResult;

	auto errorCode = assetManager.ImageLoad("paint-01-24bpp.bmp");
	if (errorCode == AssetSuite::ErrorCode::NonExistingFile)
	{
		std::cout << "ERROR: Image file doesn't exist!" << std::endl;
		return;
	}

#if DUMP_BUFFERS
	assetManager.DumpRawBuffer();
#endif

	errorCode = assetManager.ImageDecode(AssetSuite::ImageDecoders::Auto, bmpImageDesc);

#if DUMP_BUFFERS
	assetManager.DumpDecodedBuffer();
#endif

	auto getErrorCode = assetManager.ImageGet(AssetSuite::OutputFormat::RGB8, bmpResult);
	assetManager.StoreImageToFile("output.ppm", bmpResult, bmpImageDesc);

	errorCode = assetManager.MeshLoad("wavefront_sample.obj");
	if (errorCode == AssetSuite::ErrorCode::NonExistingFile)
	{
		std::cout << "ERROR: Mesh file doesn't exist!" << std::endl;
		return;
	}

	AssetSuite::MeshDescriptor meshDescriptor;
	errorCode = assetManager.MeshDecode(AssetSuite::MeshDecoders::Auto, meshDescriptor);

	std::vector<FLOAT> meshOutput;
	errorCode = assetManager.MeshGet("Plane_Plane\r", AssetSuite::MeshOutputFormat::POSITION, meshOutput);
}