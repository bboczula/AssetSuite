#pragma once

#include <AssetSuite/AssetSuite.h>

#include "../common/AssetSuite.h"

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
		Manager manager;
		LoggingState logging;
	};
}
