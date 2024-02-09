#include <CppUnitTest.h>
#include "../source/common/AssetSuite.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GeneralUnitTests
{
	TEST_CLASS(FileExtensionsTests)
	{
	public:

		TEST_METHOD(FileExtentionNotSupported)
		{
			// Test Variables
			const UINT WIDTH = 10;
			const UINT HEIGHT = 10;

			// In this test we don't need real data
			std::vector<BYTE> output;
			AssetSuite::ImageDescriptor imageDescriptor;
			AssetSuite::Manager manager;

			// This file needs to exist
			auto error = manager.ImageLoadAndDecode("test_file.xyz", imageDescriptor);
			Assert::AreEqual(true, AssetSuite::ErrorCode::FileTypeNotSupported == error);
		}
	};
}