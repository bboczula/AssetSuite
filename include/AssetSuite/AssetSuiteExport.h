#pragma once

#if defined(_WIN32)
#if defined(ASSETSUITE_BUILD_DLL) || defined(ASSETSUITE_EXPORTS)
#define ASSET_SUITE_API __declspec(dllexport)
#else
#define ASSET_SUITE_API __declspec(dllimport)
#endif
#else
#define ASSET_SUITE_API
#endif

#ifndef ASSET_SUITE_EXPORTS
#define ASSET_SUITE_EXPORTS ASSET_SUITE_API
#endif
