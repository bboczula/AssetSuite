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

	ASSET_SUITE_API void DispatchLogEvent(ContextHandle context, LogLevel level, const char* message);
}
