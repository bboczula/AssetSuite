#pragma once

#include <AssetSuite/AssetSuite.h>

#include "../common/AssetSuite.h"
#include "AssetSuiteRuntimeState.h"

namespace AssetSuite::Internal
{
	class RuntimeContext final
	{
	public:
		explicit RuntimeContext(const ContextDesc& desc);
		~RuntimeContext();

		RuntimeContext(const RuntimeContext&) = delete;
		RuntimeContext& operator=(const RuntimeContext&) = delete;

		const ContextDesc& Descriptor() const noexcept;
		Manager& LegacyManager() noexcept;
		const Manager& LegacyManager() const noexcept;
		RuntimeState::AllocatorPolicy& AllocatorPolicy() noexcept;
		RuntimeState::Diagnostics& Diagnostics() noexcept;
		const RuntimeState::Diagnostics& Diagnostics() const noexcept;
		RuntimeState::FileLoader& FileLoader() noexcept;
		RuntimeState::BlobStorage& BlobStorage() noexcept;
		const RuntimeState::BlobStorage& BlobStorage() const noexcept;
		RuntimeState::ImageStorage& ImageStorage() noexcept;
		const RuntimeState::ImageStorage& ImageStorage() const noexcept;
		RuntimeState::MeshStorage& MeshStorage() noexcept;
		const RuntimeState::MeshStorage& MeshStorage() const noexcept;
		RuntimeState::CodecRegistry& CodecRegistry() noexcept;
		const RuntimeState::CodecRegistry& CodecRegistry() const noexcept;

		void SetLoggingCallback(LoggingCallback callback, LogLevel minimumLevel, void* userData) noexcept;
		void DispatchLogEvent(LogLevel level, const char* message) const;

	private:
		struct LoggingState
		{
			LoggingCallback callback = nullptr;
			LogLevel minimumLevel = LogLevel::Info;
			void* userData = nullptr;
		};

		ContextDesc desc;
		RuntimeState state;
		Manager manager;
		LoggingState logging;
	};
}
