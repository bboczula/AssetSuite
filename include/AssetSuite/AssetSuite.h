#pragma once

#include "AssetSuiteDescriptors.h"
#include "AssetSuiteHandles.h"
#include "AssetSuiteTypes.h"

namespace AssetSuite
{
	// Creates a runtime context. Passing nullptr for desc uses default settings.
	// outContext must point to a null handle and receives ownership of the
	// created handle on success.
	ASSET_SUITE_API Result CreateContext(const ContextDesc* desc, ContextHandle* outContext);

	// Destroys a runtime context and sets the caller's handle to nullptr on
	// success. Passing nullptr or a pointer to a null handle returns
	// Result::ErrorInvalidContext.
	ASSET_SUITE_API Result DestroyContext(ContextHandle* context);

	// Registers or replaces the logging callback for a runtime context.
	// Passing nullptr for callback unregisters logging for the context.
	// Events with a level below minLevel are suppressed. The message pointer
	// passed to the callback is null-terminated UTF-8 and remains valid only
	// for the duration of the callback invocation. userData is stored without
	// interpretation and passed back unchanged. Callback invocation follows the
	// context's external synchronization requirements.
	ASSET_SUITE_API Result SetLoggingCallback(
		ContextHandle context,
		LoggingCallback callback,
		LogLevel minLevel,
		void* userData);
}
