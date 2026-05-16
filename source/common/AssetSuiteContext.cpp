#include "AssetSuiteContext.h"

#include "../runtime/AssetSuiteRuntime.h"

AssetSuite::AssetSuiteContext_t::AssetSuiteContext_t(const ContextDesc& desc)
	: runtime(std::make_unique<Internal::RuntimeContext>(desc))
{
}

AssetSuite::AssetSuiteContext_t::~AssetSuiteContext_t() = default;

AssetSuite::Internal::RuntimeContext& AssetSuite::AssetSuiteContext_t::Runtime()
{
	return *runtime;
}

const AssetSuite::Internal::RuntimeContext& AssetSuite::AssetSuiteContext_t::Runtime() const
{
	return *runtime;
}

void AssetSuite::DispatchLogEvent(ContextHandle context, LogLevel level, const char* message)
{
	if (!context)
	{
		return;
	}

	context->Runtime().DispatchLogEvent(level, message);
}
