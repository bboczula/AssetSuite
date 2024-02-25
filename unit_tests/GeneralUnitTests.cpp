#include <CppUnitTest.h>
#include "../source/common/AssetSuite.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GeneralUnitTests
{
	TEST_CLASS(FileExtensionsTests)
	{
	public:

		TEST_METHOD(FileExtensionNotSupported)
		{
			// In this test we don't need real data
			std::vector<BYTE> output;
			AssetSuite::ImageDescriptor imageDescriptor;
			AssetSuite::Manager manager;

			// This file needs to exist
			manager.ImageLoad("test_file.xyz");
			auto error = manager.ImageDecode(AssetSuite::ImageDecoders::Auto, imageDescriptor);
			Assert::AreEqual(true, AssetSuite::ErrorCode::FileTypeNotSupported == error);
		}

		TEST_METHOD(RawBufferIsEmpty)
		{
			// In this test we don't need real data
			std::vector<BYTE> output;
			AssetSuite::ImageDescriptor imageDescriptor;
			AssetSuite::Manager manager;

			auto error = manager.ImageDecode(AssetSuite::ImageDecoders::Auto, imageDescriptor);
			Assert::AreEqual(true, AssetSuite::ErrorCode::RawBufferIsEmpty == error);
		}

		TEST_METHOD(DecodedBufferIsEmpty)
		{
			// In this test we don't need real data
			std::vector<BYTE> output;
			AssetSuite::ImageDescriptor imageDescriptor;
			AssetSuite::Manager manager;

			auto error = manager.ImageGet(AssetSuite::OutputFormat::RGB8, output);
			Assert::AreEqual(true, AssetSuite::ErrorCode::DecodedBufferIsEmpty == error);
		}
	};
}