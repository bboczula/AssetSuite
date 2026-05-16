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

	// Registers, replaces, or unregisters the logging callback for a runtime
	// context. A successful registration replaces the previous callback,
	// minimum level, and userData together. Passing nullptr for callback
	// unregisters logging and discards the previous callback state.
	//
	// LogLevel values are ordered from least to most severe. Events with a
	// level numerically lower than minLevel are suppressed, and events with a
	// level equal to or greater than minLevel are delivered when a callback is
	// registered.
	//
	// The callback does not take ownership of message. The message pointer is
	// null-terminated UTF-8 and remains valid only for the duration of the
	// callback invocation. userData is stored without interpretation and passed
	// back unchanged. Callback invocation follows the context's external
	// synchronization requirements.
	ASSET_SUITE_API Result SetLoggingCallback(
		ContextHandle context,
		LoggingCallback callback,
		LogLevel minLevel,
		void* userData);
}
