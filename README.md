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

`AssetSuite::DestroyContext` takes a pointer to the caller's handle so the SDK
can set it to `nullptr` after successful destruction. Calling
`AssetSuite::DestroyContext(nullptr)` or passing a pointer to a null context
handle, including a repeated destroy through the same handle variable, returns
`AssetSuite::Result::ErrorInvalidContext` instead of causing undefined behavior.

## Usage

The current 2.0 public SDK headers expose the stable type and version contract used by external consumers:

```cpp
#include <AssetSuite/AssetSuite.h>

#include <cstdint>

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
