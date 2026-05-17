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
	// synchronization requirements. Passing a null context returns
	// Result::ErrorInvalidContext.
	ASSET_SUITE_API Result SetLoggingCallback(
		ContextHandle context,
		LoggingCallback callback,
		LogLevel minLevel,
		void* userData);

	// Loads a file into a runtime-owned blob. outBlob must point to a null
	// handle and receives ownership of the created blob handle on success.
	//
	// The blob bytes and source metadata are owned by the context. The caller
	// retains ownership of filePath and may release or mutate that string after
	// the call returns. A returned blob remains valid until ReleaseBlob is
	// called for the same context, or until the owning context is destroyed.
	// Blob operations follow the context's external synchronization
	// requirements.
	//
	// Passing a null context returns Result::ErrorInvalidContext. Passing a
	// null filePath, null outBlob, or non-null *outBlob returns
	// Result::ErrorInvalidArgument. Missing files return
	// Result::ErrorFileNotFound. Existing files that cannot be opened or read
	// return Result::ErrorIoFailure.
	ASSET_SUITE_API Result LoadFile(ContextHandle context, const char* filePath, BlobHandle* outBlob);

	// Releases a runtime-owned blob and sets the caller's handle to nullptr on
	// success. Memory and metadata behind the blob are invalid immediately
	// after a successful release.
	//
	// The blob must have been created by LoadFile on the same context. Passing
	// a null context returns Result::ErrorInvalidContext. Passing nullptr, a
	// pointer to a null blob handle, a stale handle, or a handle from another
	// context returns Result::ErrorInvalidHandle. On failure, the caller's blob
	// value is preserved.
	ASSET_SUITE_API Result ReleaseBlob(ContextHandle context, BlobHandle* blob);
}
