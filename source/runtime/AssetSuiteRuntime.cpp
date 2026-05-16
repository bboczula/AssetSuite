#include "AssetSuiteRuntime.h"

AssetSuite::Internal::RuntimeContext::RuntimeContext(const ContextDesc& desc)
	: desc(desc)
	, manager()
	, logging()
{
}

AssetSuite::Internal::RuntimeContext::~RuntimeContext() = default;

const AssetSuite::ContextDesc& AssetSuite::Internal::RuntimeContext::Descriptor() const noexcept
{
	return desc;
}

AssetSuite::Manager& AssetSuite::Internal::RuntimeContext::LegacyManager() noexcept
{
	return manager;
}

const AssetSuite::Manager& AssetSuite::Internal::RuntimeContext::LegacyManager() const noexcept
{
	return manager;
}

void AssetSuite::Internal::RuntimeContext::SetLoggingCallback(
	LoggingCallback callback,
	LogLevel minimumLevel,
	void* userData) noexcept
{
	logging.callback = callback;
	logging.minimumLevel = minimumLevel;
	logging.userData = userData;
}

void AssetSuite::Internal::RuntimeContext::DispatchLogEvent(LogLevel level, const char* message) const
{
	if (!logging.callback || !message)
	{
		return;
	}

	if (static_cast<uint32_t>(level) < static_cast<uint32_t>(logging.minimumLevel))
	{
		return;
	}

	logging.callback(level, message, logging.userData);
}
