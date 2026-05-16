#pragma once

#include <memory>

#include <AssetSuite/AssetSuite.h>

namespace AssetSuite
{
	namespace Internal
	{
		class RuntimeContext;

		Result CreateContextHandle(const ContextDesc& desc, ContextHandle* outContext) noexcept;
		Result DestroyContextHandle(ContextHandle* context) noexcept;
	}

	struct AssetSuiteContext_t
	{
		explicit AssetSuiteContext_t(const ContextDesc& desc);
		~AssetSuiteContext_t();

		Internal::RuntimeContext& Runtime();
		const Internal::RuntimeContext& Runtime() const;

	private:
		std::unique_ptr<Internal::RuntimeContext> runtime;
	};

	struct RuntimeSmokeStatus
	{
		uint32_t descriptorStructSize;
		uint32_t descriptorFlags;
		uint32_t diagnosticsEntryCount;
		bool hasBmpDecoder;
		bool hasPngDecoder;
		bool hasWavefrontDecoder;
		bool rejectsImageSentinels;
		bool rejectsMeshSentinels;
	};

	ASSET_SUITE_API void DispatchLogEvent(ContextHandle context, LogLevel level, const char* message);
	ASSET_SUITE_API Result CaptureRuntimeSmokeStatus(ContextHandle context, RuntimeSmokeStatus* outStatus);
	ASSET_SUITE_API Result AddRuntimeSmokeDiagnostic(ContextHandle context);
	ASSET_SUITE_API Result ClearRuntimeSmokeDiagnostics(ContextHandle context);
}
