#pragma once

#include "AssetSuite.h"

namespace AssetSuite
{
	struct AssetSuiteContext_t
	{
		explicit AssetSuiteContext_t(const ContextDesc& desc)
			: desc(desc)
			, manager()
			, loggingCallback(nullptr)
			, minimumLogLevel(LogLevel::Info)
			, loggingUserData(nullptr)
		{
		}

		ContextDesc desc;
		Manager manager;
		LoggingCallback loggingCallback;
		LogLevel minimumLogLevel;
		void* loggingUserData;
	};

	inline void DispatchLogEvent(ContextHandle context, LogLevel level, const char* message)
	{
		if (!context || !context->loggingCallback || !message)
		{
			return;
		}

		if (static_cast<uint32_t>(level) < static_cast<uint32_t>(context->minimumLogLevel))
		{
			return;
		}

		context->loggingCallback(level, message, context->loggingUserData);
	}
}
