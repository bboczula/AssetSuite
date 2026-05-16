#pragma once

#include "AssetSuiteDescriptors.h"
#include "AssetSuiteHandles.h"
#include "AssetSuiteTypes.h"

namespace AssetSuite
{
	// Creates a runtime context. Passing nullptr for desc uses default settings.
	// outContext must not be null and receives ownership of the created handle
	// on success.
	ASSET_SUITE_API Result CreateContext(const ContextDesc* desc, ContextHandle* outContext);

	// Destroys a runtime context and sets the caller's handle to nullptr on
	// success. Passing nullptr or a pointer to a null handle returns
	// Result::ErrorInvalidContext.
	ASSET_SUITE_API Result DestroyContext(ContextHandle* context);
}
