#pragma warning (disable : 4251)
#include <iostream>
#include <vector>
#include "../common/AssetSuite.h"

#define DUMP_BUFFERS 0

// Internal legacy demo for the pre-2.0 implementation API.
// This intentionally uses source/common headers and must not be treated as the
// public SDK integration example. External consumers should include
// <AssetSuite/AssetSuite.h> from the installed SDK header root instead.
void main()
{
	AssetSuite::Manager assetManager;
	std::cout << "Hello, AssetSuite legacy internal demo!" << std::endl;

	AssetSuite::ImageDescriptor imageDescriptor = {};
	auto errorCode = assetManager.ImageLoadAndDecode("girl_with_pearl_earring.bmp");

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
