#include "AssetSuiteRuntime.h"

AssetSuite::Internal::RuntimeContext::RuntimeContext(const ContextDesc& desc)
	: desc(desc)
	, state()
	, manager(state)
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

AssetSuite::Internal::RuntimeState::AllocatorPolicy&
AssetSuite::Internal::RuntimeContext::AllocatorPolicy() noexcept
{
	return state.allocatorPolicy;
}

AssetSuite::Internal::RuntimeState::Diagnostics&
AssetSuite::Internal::RuntimeContext::Diagnostics() noexcept
{
	return state.diagnostics;
}

const AssetSuite::Internal::RuntimeState::Diagnostics&
AssetSuite::Internal::RuntimeContext::Diagnostics() const noexcept
{
	return state.diagnostics;
}

AssetSuite::Internal::RuntimeState::FileLoader&
AssetSuite::Internal::RuntimeContext::FileLoader() noexcept
{
	return state.fileLoader;
}

AssetSuite::Internal::RuntimeState::BlobStorage&
AssetSuite::Internal::RuntimeContext::BlobStorage() noexcept
{
	return state.blobStorage;
}

const AssetSuite::Internal::RuntimeState::BlobStorage&
AssetSuite::Internal::RuntimeContext::BlobStorage() const noexcept
{
	return state.blobStorage;
}

AssetSuite::Internal::RuntimeState::CodecRegistry&
AssetSuite::Internal::RuntimeContext::CodecRegistry() noexcept
{
	return state.codecRegistry;
}

const AssetSuite::Internal::RuntimeState::CodecRegistry&
AssetSuite::Internal::RuntimeContext::CodecRegistry() const noexcept
{
	return state.codecRegistry;
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
