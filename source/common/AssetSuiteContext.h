#pragma once

#include "AssetSuite.h"

namespace AssetSuite
{
	struct AssetSuiteContext_t
	{
		explicit AssetSuiteContext_t(const ContextDesc& desc);

		ContextDesc desc;
		Manager manager;
	};
}
