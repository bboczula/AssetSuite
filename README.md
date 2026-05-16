# AssetSuite

## Overview

AssetSuite is a C++ asset loading SDK for image and mesh data. The 2.0 SDK surface is designed as a small public boundary for engine and tool integration: consumers include stable public headers, receive SDK result codes, and exchange plain descriptor structs and opaque handles instead of internal implementation classes.

## Public SDK Surface

External consumers should include the 2.0 public header from the installed include root:

```cpp
#include <AssetSuite/AssetSuite.h>
```

The public headers are provided under `include/AssetSuite` and are installed under `inc/AssetSuite`. They define:

- `AssetSuite::Result` and `AssetSuite::Version`
- `AssetSuite::GetVersion` and `AssetSuite::GetResultString`
- `AssetSuite::LogLevel`, `AssetSuite::LoggingCallback`, and `AssetSuite::SetLoggingCallback`
- opaque handle types such as `AssetSuite::ContextHandle`, `AssetSuite::BlobHandle`, `AssetSuite::ImageHandle`, and `AssetSuite::MeshHandle`
- plain descriptor structs such as `AssetSuite::ContextDesc`, `AssetSuite::BlobDesc`, `AssetSuite::ImageDesc`, and `AssetSuite::MeshDesc`
- SDK enums such as `AssetSuite::AssetFormat`, `AssetSuite::PixelFormat`, and `AssetSuite::MeshAttributeFlags`

Public headers do not require `Windows.h`, STL containers, filesystem path types, or internal implementation classes.

## Context Lifecycle Contract

The 2.0 runtime is entered through an opaque `AssetSuite::ContextHandle`.
Context ownership belongs to the caller after successful creation and must be
released with `AssetSuite::DestroyContext`.

`AssetSuite::CreateContext(nullptr, &context)` is valid and creates a context
with default settings. A non-null `AssetSuite::ContextDesc` must use
`structSize == sizeof(AssetSuite::ContextDesc)` and `flags == 0`. Smaller,
larger, or otherwise mismatched descriptor sizes are rejected with
`AssetSuite::Result::ErrorInvalidArgument` for now. Unknown context flag bits are
also rejected with `AssetSuite::Result::ErrorInvalidArgument`.
The output handle passed to `AssetSuite::CreateContext` must point to a null
handle; passing a pointer to an already-owned context returns
`AssetSuite::Result::ErrorInvalidArgument` and leaves that handle unchanged.

`AssetSuite::DestroyContext` takes a pointer to the caller's handle so the SDK
can set it to `nullptr` after successful destruction. Calling
`AssetSuite::DestroyContext(nullptr)` or passing a pointer to a null context
handle, including a repeated destroy through the same handle variable, returns
`AssetSuite::Result::ErrorInvalidContext` instead of causing undefined behavior.

## Logging Callback Contract

Logging is configured per `AssetSuite::ContextHandle` with
`AssetSuite::SetLoggingCallback`. A successful call replaces the context's
previous callback pointer, minimum enabled `AssetSuite::LogLevel`, and opaque
`userData` pointer together. Passing `nullptr` for the callback unregisters
logging for that context and discards the previous callback state.

`AssetSuite::LogLevel` values are ordered from least to most severe:
`Trace`, `Debug`, `Info`, `Warning`, `Error`, and `Fatal`. The configured
minimum level is inclusive: events below it are suppressed, while events equal
to or above it are delivered when a callback is registered.

The callback receives a null-terminated UTF-8 message pointer and the exact
`userData` pointer supplied during registration. The SDK does not take ownership
of `userData`, does not interpret it, and does not manage its lifetime. The
message pointer is valid only for the duration of the callback invocation.
Callback invocation follows the same external synchronization expectations as
other operations on the context.

## Usage

The current 2.0 public SDK headers expose the stable type, version, and context
lifecycle contract used by external consumers:

```cpp
#include <AssetSuite/AssetSuite.h>

#include <cstdint>

struct AppLogger
{
	int messagesSeen;
};

void OnAssetSuiteLog(AssetSuite::LogLevel level, const char* message, void* userData)
{
	AppLogger* logger = static_cast<AppLogger*>(userData);
	++logger->messagesSeen;

	(void)level;
	(void)message;
}

int main()
{
	AssetSuite::Version version = {};
	AssetSuite::Result result = AssetSuite::GetVersion(&version);

	if (result != AssetSuite::Result::Success)
	{
		const char* message = AssetSuite::GetResultString(result);
		return message != nullptr ? 1 : 2;
	}

	AssetSuite::ContextHandle context = nullptr;
	AssetSuite::BlobHandle blob = nullptr;
	AssetSuite::ImageHandle image = nullptr;
	AssetSuite::MeshHandle mesh = nullptr;

	AssetSuite::ContextDesc contextDesc = { sizeof(AssetSuite::ContextDesc), 0 };
	result = AssetSuite::CreateContext(&contextDesc, &context);
	if (result != AssetSuite::Result::Success)
	{
		const char* message = AssetSuite::GetResultString(result);
		return message != nullptr ? 3 : 4;
	}

	AppLogger logger = {};
	result = AssetSuite::SetLoggingCallback(
		context,
		&OnAssetSuiteLog,
		AssetSuite::LogLevel::Warning,
		&logger);
	if (result != AssetSuite::Result::Success)
	{
		const char* message = AssetSuite::GetResultString(result);
		return message != nullptr ? 5 : 6;
	}

	result = AssetSuite::SetLoggingCallback(
		context,
		nullptr,
		AssetSuite::LogLevel::Trace,
		nullptr);
	if (result != AssetSuite::Result::Success)
	{
		const char* message = AssetSuite::GetResultString(result);
		return message != nullptr ? 7 : 8;
	}

	result = AssetSuite::DestroyContext(&context);
	if (result != AssetSuite::Result::Success)
	{
		const char* message = AssetSuite::GetResultString(result);
		return message != nullptr ? 9 : 10;
	}

	AssetSuite::BlobDesc blobDesc = {
		sizeof(AssetSuite::BlobDesc),
		0,
		AssetSuite::AssetFormat::Unknown,
		0
	};
	AssetSuite::ImageDesc imageDesc = {
		sizeof(AssetSuite::ImageDesc),
		0,
		0,
		AssetSuite::PixelFormat::Unknown,
		0,
		0
	};
	AssetSuite::MeshDesc meshDesc = {
		sizeof(AssetSuite::MeshDesc),
		0,
		0,
		0,
		static_cast<uint32_t>(AssetSuite::MeshAttributeFlags::None)
	};

	return context || blob || image || mesh ||
		contextDesc.structSize == 0 ||
		blobDesc.structSize == 0 ||
		imageDesc.structSize == 0 ||
		meshDesc.structSize == 0;
}
```

Asset loading entry points will build on these handle and descriptor types as the 2.0 API is expanded.

## Integration

Build the project with Premake-generated Visual Studio projects. The package output contains Debug and Release binaries plus the public SDK headers under `bin/<Config>/inc/AssetSuite`.

The installed public header surface is validated by `PublicHeaderCompile`, which compiles an external-consumer translation unit and checks that installed headers do not expose platform-specific, legacy, or STL-owning API types.

## Installation

AssetSuite can be built from source using Premake or consumed from release artifacts when available.

## License

This library is provided as is, and it uses the MIT license.
