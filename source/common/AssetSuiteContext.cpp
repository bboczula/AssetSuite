#include "AssetSuiteContext.h"

#include "../runtime/AssetSuiteRuntime.h"

#include <new>

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

AssetSuite::Result AssetSuite::Internal::CreateContextHandle(
	const ContextDesc& desc,
	ContextHandle* outContext) noexcept
{
	try
	{
		std::unique_ptr<AssetSuiteContext_t> context = std::make_unique<AssetSuiteContext_t>(desc);
		*outContext = context.release();
	}
	catch (const std::bad_alloc&)
	{
		return Result::ErrorOutOfMemory;
	}
	catch (...)
	{
		return Result::ErrorUnknown;
	}

	return Result::Success;
}

AssetSuite::Result AssetSuite::Internal::DestroyContextHandle(ContextHandle* context) noexcept
{
	if (!context || !*context)
	{
		return Result::ErrorInvalidContext;
	}

	try
	{
		delete *context;
		*context = nullptr;
	}
	catch (...)
	{
		return Result::ErrorUnknown;
	}

	return Result::Success;
}

void AssetSuite::DispatchLogEvent(ContextHandle context, LogLevel level, const char* message)
{
	if (!context)
	{
		return;
	}

	context->Runtime().DispatchLogEvent(level, message);
}
