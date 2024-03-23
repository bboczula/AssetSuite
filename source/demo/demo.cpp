#pragma warning (disable : 4251)
#include <iostream>
#include <vector>
#include "../common/AssetSuite.h"

#define DUMP_BUFFERS 0

void main()
{
	AssetSuite::Manager assetManager;
	std::cout << "Hello, AssetSuite!" << std::endl;

	AssetSuite::ImageDescriptor imageDescriptor = {};
	auto errorCode = assetManager.ImageLoadAndDecode("paint-01-24bpp.bmp");

#if DUMP_BUFFERS
	assetManager.DumpRawBuffer();
	assetManager.DumpDecodedBuffer();
#endif

	std::vector<BYTE> imageOutput;
	errorCode = assetManager.ImageGet(AssetSuite::OutputFormat::RGB8, imageOutput, imageDescriptor);
	assetManager.StoreImageToFile("output.ppm", imageOutput, imageDescriptor);

	AssetSuite::MeshDescriptor meshDescriptor;
	errorCode = assetManager.MeshLoadAndDecode("wavefront_sample.obj");

	std::vector<FLOAT> meshOutput;
	errorCode = assetManager.MeshGet("Plane_Plane\r", AssetSuite::MeshOutputFormat::POSITION, meshOutput, meshDescriptor);
}