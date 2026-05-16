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
}
