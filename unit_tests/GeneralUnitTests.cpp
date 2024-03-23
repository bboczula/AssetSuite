#pragma warning (disable : 4251)
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
			AssetSuite::Manager manager;

			// This file needs to exist
			manager.ImageLoad("test_file.xyz");
			auto error = manager.ImageDecode(AssetSuite::ImageDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::FileTypeNotSupported == error);
		}

		TEST_METHOD(RawBufferIsEmpty)
		{
			// In this test we don't need real data
			std::vector<BYTE> output;
			AssetSuite::Manager manager;

			auto error = manager.ImageDecode(AssetSuite::ImageDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::RawBufferIsEmpty == error);
		}

		TEST_METHOD(DecodedBufferIsEmpty)
		{
			// In this test we don't need real data
			std::vector<BYTE> output;
			AssetSuite::ImageDescriptor imageDescriptor;
			AssetSuite::Manager manager;

			auto error = manager.ImageGet(AssetSuite::OutputFormat::RGB8, output, imageDescriptor);
			Assert::AreEqual(true, AssetSuite::ErrorCode::DecodedBufferIsEmpty == error);
		}
	};

	TEST_CLASS(MeshTests)
	{
	public:
		TEST_METHOD(OpeningNonExistingFile)
		{
			std::vector<FLOAT> output;
			AssetSuite::Manager manager;

			auto error = manager.MeshLoad("non-existing-mesh.obj");
			Assert::AreEqual(true, AssetSuite::ErrorCode::NonExistingFile == error);
		}

		TEST_METHOD(FileTypeNotSupported)
		{
			std::vector<FLOAT> output;
			AssetSuite::Manager manager;

			manager.MeshLoad("test_file.xyz");
			auto error = manager.MeshDecode(AssetSuite::MeshDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::FileTypeNotSupported == error);
		}
		TEST_METHOD(MeshPostion)
		{
			AssetSuite::Manager manager;

			auto error = manager.MeshLoad("test_mesh.obj");
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			error = manager.MeshDecode(AssetSuite::MeshDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			std::vector<FLOAT> output;
			AssetSuite::MeshDescriptor descriptor;
			error = manager.MeshGet("TriangleOne_Mesh\r", AssetSuite::MeshOutputFormat::POSITION, output, descriptor);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			// Three vertices, each have four components
			Assert::AreEqual((size_t)12, output.size());

			// Check if the data is correct
			Assert::AreEqual(-1.0f, output[0]);
			Assert::AreEqual(0.0f, output[1]);
			Assert::AreEqual(1.0f, output[2]);
			Assert::AreEqual(1.0f, output[3]);

			Assert::AreEqual(1.0f, output[4]);
			Assert::AreEqual(0.0f, output[5]);
			Assert::AreEqual(1.0f, output[6]);
			Assert::AreEqual(1.0f, output[7]);

			Assert::AreEqual(-1.0f, output[8]);
			Assert::AreEqual(0.0f, output[9]);
			Assert::AreEqual(-1.0f, output[10]);
			Assert::AreEqual(1.0f, output[11]);
		}

		TEST_METHOD(MeshNormal)
		{
			AssetSuite::Manager manager;

			auto error = manager.MeshLoad("test_mesh.obj");
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			error = manager.MeshDecode(AssetSuite::MeshDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			std::vector<FLOAT> output;
			AssetSuite::MeshDescriptor descriptor;
			error = manager.MeshGet("TriangleOne_Mesh\r", AssetSuite::MeshOutputFormat::NORMAL, output, descriptor);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			// Three vertices, each have four components
			Assert::AreEqual((size_t)12, output.size());

			// Check if the data is correct
			Assert::AreEqual(0.0f, output[0]);
			Assert::AreEqual(1.0f, output[1]);
			Assert::AreEqual(0.0f, output[2]);
			Assert::AreEqual(0.0f, output[3]);

			Assert::AreEqual(0.0f, output[4]);
			Assert::AreEqual(1.0f, output[5]);
			Assert::AreEqual(0.0f, output[6]);
			Assert::AreEqual(0.0f, output[7]);

			Assert::AreEqual(0.0f, output[8]);
			Assert::AreEqual(1.0f, output[9]);
			Assert::AreEqual(0.0f, output[10]);
			Assert::AreEqual(0.0f, output[11]);
		}

		TEST_METHOD(MeshTangent)
		{
			AssetSuite::Manager manager;

			auto error = manager.MeshLoad("tangent_mesh_1.obj");
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			
			error = manager.MeshDecode(AssetSuite::MeshDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			std::vector<FLOAT> output;
			AssetSuite::MeshDescriptor descriptor;
			error = manager.MeshGet("TriangleOne_Mesh\r", AssetSuite::MeshOutputFormat::TANGENT, output, descriptor);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			// Three vertices, each have four components
			Assert::AreEqual((size_t)12, output.size());

			// Check if the data is correct
			Assert::AreEqual(2.0f, output[0]);
			Assert::AreEqual(0.0f, output[1]);
			Assert::AreEqual(0.0f, output[2]);
			Assert::AreEqual(0.0f, output[3]);

			Assert::AreEqual(2.0f, output[4]);
			Assert::AreEqual(0.0f, output[5]);
			Assert::AreEqual(0.0f, output[6]);
			Assert::AreEqual(0.0f, output[7]);

			Assert::AreEqual(2.0f, output[8]);
			Assert::AreEqual(0.0f, output[9]);
			Assert::AreEqual(0.0f, output[10]);
			Assert::AreEqual(0.0f, output[11]);
		}

		TEST_METHOD(MeshTexCoords)
		{
			AssetSuite::Manager manager;

			auto error = manager.MeshLoad("test_mesh.obj");
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			error = manager.MeshDecode(AssetSuite::MeshDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			std::vector<FLOAT> output;
			AssetSuite::MeshDescriptor descriptor;
			error = manager.MeshGet("TriangleOne_Mesh\r", AssetSuite::MeshOutputFormat::TEXCOORD, output, descriptor);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			// Three vertices, each have two components
			Assert::AreEqual((size_t)6, output.size());

			// Check if the data is correct
			Assert::AreEqual(-2.0f, output[0]);
			Assert::AreEqual(-1.0f, output[1]);

			Assert::AreEqual(3.0f, output[2]);
			Assert::AreEqual(-2.0f, output[3]);

			Assert::AreEqual(-2.0f, output[4]);
			Assert::AreEqual(3.0f, output[5]);
		}
	};
}