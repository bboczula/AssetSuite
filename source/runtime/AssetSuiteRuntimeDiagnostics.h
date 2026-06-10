#pragma once

namespace AssetSuite::Internal::Diagnostics
{
	inline constexpr const char* UnsupportedImageFormat =
		"ASSET_UNSUPPORTED_FORMAT: Image blob format is not supported.";
	inline constexpr const char* ImageBlobTooSmall =
		"ASSET_MALFORMED_DATA: Image blob is too small for the selected decoder.";
	inline constexpr const char* ImageDecoderMissing =
		"ASSET_UNSUPPORTED_FORMAT: Image decoder is not registered.";
	inline constexpr const char* ImageDecodeRejected =
		"ASSET_MALFORMED_DATA: Image decoder rejected malformed data.";

	inline constexpr const char* UnsupportedMeshFormat =
		"ASSET_UNSUPPORTED_FORMAT: Mesh blob format is not supported.";
	inline constexpr const char* MeshBlobTooSmall =
		"ASSET_MALFORMED_DATA: Mesh blob is too small for the selected decoder.";
	inline constexpr const char* MeshDecoderMissing =
		"ASSET_UNSUPPORTED_FORMAT: Mesh decoder is not registered.";
	inline constexpr const char* MeshDecodeRejected =
		"ASSET_MALFORMED_DATA: Mesh decoder rejected malformed data.";

	inline constexpr const char* BlobLoadFailed =
		"ASSET_FILE_LOAD_FAILED: Failed to load blob source file.";
	inline constexpr const char* ImageLoadFailed =
		"ASSET_FILE_LOAD_FAILED: Failed to load image source file.";
	inline constexpr const char* MeshLoadFailed =
		"ASSET_FILE_LOAD_FAILED: Failed to load mesh source file.";
}
