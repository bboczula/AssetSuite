#pragma once

#include <cstdint>

#include "AssetSuiteExport.h"

namespace AssetSuite
{
	enum class Result : int32_t
	{
		Success = 0,
		WarningUnsupportedChunk = 1,

		ErrorInvalidArgument = -1,
		ErrorUnsupportedFormat = -2,
		ErrorFileNotFound = -3,
		ErrorDecodeFailed = -4,
		ErrorOutputBufferTooSmall = -5,
		ErrorOutOfMemory = -6,
		ErrorInvalidContext = -7,
		ErrorInvalidHandle = -8,
		ErrorIoFailure = -9,
		ErrorUnknown = -1000
	};

	struct Version
	{
		uint16_t major;
		uint16_t minor;
		uint16_t patch;
		uint16_t reserved;
	};

	enum class PixelFormat : uint32_t
	{
		Unknown = 0,
		RGB8 = 1,
		RGBA8 = 2
	};

	enum class AssetFormat : uint32_t
	{
		Unknown = 0,
		BMP = 1,
		PNG = 2,
		PPM = 3,
		WavefrontObj = 4
	};

	enum class MeshAttributeFlags : uint32_t
	{
		None = 0,
		Position = 1u << 0,
		Normal = 1u << 1,
		Tangent = 1u << 2,
		TexCoord = 1u << 3,
		Index = 1u << 4
	};

	// Logging severity values are ordered from least to most severe. A minimum
	// enabled level suppresses events with numerically lower values.
	enum class LogLevel : uint32_t
	{
		Trace = 0,
		Debug = 1,
		Info = 2,
		Warning = 3,
		Error = 4,
		Fatal = 5
	};

	// The message pointer is null-terminated UTF-8 and is valid only for the
	// duration of the callback invocation. userData is the opaque pointer
	// supplied to SetLoggingCallback.
	typedef void (*LoggingCallback)(LogLevel level, const char* message, void* userData);

	ASSET_SUITE_API Result GetVersion(Version* outVersion);
	ASSET_SUITE_API const char* GetResultString(Result result);
}
