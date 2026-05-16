#include "AssetSuiteRuntimeState.h"

#include "../wavefront/ModelLoader.h"
#include "../bmp/BmpDecoder.h"
#include "../png/PngDecoder.h"
#include "../ppm/PpmEncoder.h"
#include "../bypass/BypassEncoder.h"

AssetSuite::Internal::RuntimeState::RuntimeState()
	: modelLoader(std::make_unique<ModelLoader>())
	, bmpDecoder(std::make_unique<BmpDecoder>())
	, pngDecoder(std::make_unique<PngDecoder>())
	, ppmEncoder(std::make_unique<PpmEncoder>())
	, bypassEncoder(std::make_unique<BypassEncoder>())
	, imageDecoders()
	, meshDecoders()
{
	imageDecoders[static_cast<size_t>(ImageDecoders::BMP)] = bmpDecoder.get();
	imageDecoders[static_cast<size_t>(ImageDecoders::PNG)] = pngDecoder.get();

	meshDecoders[static_cast<size_t>(MeshDecoders::WAVEFRONT)] = modelLoader.get();
}

AssetSuite::Internal::RuntimeState::~RuntimeState() = default;
