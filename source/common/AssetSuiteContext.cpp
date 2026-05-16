#include "AssetSuiteContext.h"

void AssetSuite::DispatchLogEvent(ContextHandle context, LogLevel level, const char* message)
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
