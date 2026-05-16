#pragma warning (disable : 4251)
#include <CppUnitTest.h>
#include "../source/common/AssetSuite.h"
#include "../source/common/AssetSuiteContext.h"
#include "../source/runtime/AssetSuiteRuntime.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GeneralUnitTests
{
	TEST_CLASS(PublicApiTests)
	{
	public:
		TEST_METHOD(GetVersionReturnsSdkVersion)
		{
			AssetSuite::Version version = {};

			auto result = AssetSuite::GetVersion(&version);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);
			Assert::AreEqual(static_cast<uint16_t>(2), version.major);
			Assert::AreEqual(static_cast<uint16_t>(0), version.minor);
			Assert::AreEqual(static_cast<uint16_t>(0), version.patch);
			Assert::AreEqual(static_cast<uint16_t>(0), version.reserved);
		}

		TEST_METHOD(GetVersionReturnsInvalidArgumentForNullOutput)
		{
			auto result = AssetSuite::GetVersion(nullptr);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidArgument == result);
		}

		TEST_METHOD(GetResultStringReturnsStableStringsForPublicResults)
		{
			AssertResultString(AssetSuite::Result::Success, "Success");
			AssertResultString(AssetSuite::Result::WarningUnsupportedChunk, "Warning: unsupported chunk");
			AssertResultString(AssetSuite::Result::ErrorInvalidArgument, "Error: invalid argument");
			AssertResultString(AssetSuite::Result::ErrorUnsupportedFormat, "Error: unsupported format");
			AssertResultString(AssetSuite::Result::ErrorFileNotFound, "Error: file not found");
			AssertResultString(AssetSuite::Result::ErrorDecodeFailed, "Error: decode failed");
			AssertResultString(AssetSuite::Result::ErrorOutputBufferTooSmall, "Error: output buffer too small");
			AssertResultString(AssetSuite::Result::ErrorOutOfMemory, "Error: out of memory");
			AssertResultString(AssetSuite::Result::ErrorInvalidContext, "Error: invalid context");
			AssertResultString(AssetSuite::Result::ErrorUnknown, "Error: unknown");
		}

		TEST_METHOD(GetResultStringReturnsFallbackForUnknownResult)
		{
			const auto unknownResult = static_cast<AssetSuite::Result>(1234);

			Assert::AreEqual("Unknown result", AssetSuite::GetResultString(unknownResult));
		}

		TEST_METHOD(CreateContextAcceptsNullDescriptor)
		{
			AssetSuite::ContextHandle context = nullptr;

			const auto result = AssetSuite::CreateContext(nullptr, &context);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);
			Assert::IsNotNull(context);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(CreateContextAcceptsDefaultDescriptor)
		{
			AssetSuite::ContextDesc desc = { sizeof(AssetSuite::ContextDesc), 0 };
			AssetSuite::ContextHandle context = nullptr;

			const auto result = AssetSuite::CreateContext(&desc, &context);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);
			Assert::IsNotNull(context);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(CreateContextInitializesRuntimeServices)
		{
			AssetSuite::ContextDesc desc = { sizeof(AssetSuite::ContextDesc), 0 };
			AssetSuite::ContextHandle context = nullptr;

			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(&desc, &context));

			const auto& runtime = context->Runtime();
			Assert::AreEqual(desc.structSize, runtime.Descriptor().structSize);
			Assert::AreEqual(desc.flags, runtime.Descriptor().flags);
			Assert::IsNotNull(runtime.CodecRegistry().FindImageDecoder(AssetSuite::ImageDecoders::BMP));
			Assert::IsNotNull(runtime.CodecRegistry().FindImageDecoder(AssetSuite::ImageDecoders::PNG));
			Assert::IsNotNull(runtime.CodecRegistry().FindMeshDecoder(AssetSuite::MeshDecoders::WAVEFRONT));
			Assert::AreEqual(static_cast<size_t>(0), runtime.Diagnostics().Entries().size());

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(CreateContextRejectsNullOutput)
		{
			const auto result = AssetSuite::CreateContext(nullptr, nullptr);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidArgument == result);
		}

		TEST_METHOD(CreateContextRejectsInvalidDescriptorSize)
		{
			AssetSuite::ContextDesc desc = { sizeof(AssetSuite::ContextDesc) - 1, 0 };
			AssetSuite::ContextHandle context = nullptr;

			const auto result = AssetSuite::CreateContext(&desc, &context);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidArgument == result);
			Assert::IsNull(context);
		}

		TEST_METHOD(CreateContextRejectsReservedFlags)
		{
			AssetSuite::ContextDesc desc = { sizeof(AssetSuite::ContextDesc), 1 };
			AssetSuite::ContextHandle context = nullptr;

			const auto result = AssetSuite::CreateContext(&desc, &context);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidArgument == result);
			Assert::IsNull(context);
		}

		TEST_METHOD(CreateContextRejectsNonNullOutputAndPreservesHandle)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			AssetSuite::ContextHandle originalContext = context;

			const auto result = AssetSuite::CreateContext(nullptr, &context);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidArgument == result);
			Assert::IsTrue(originalContext == context);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(DestroyContextReleasesValidContextAndClearsHandle)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			Assert::IsNotNull(context);

			const auto result = AssetSuite::DestroyContext(&context);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);
			Assert::IsNull(context);
		}

		TEST_METHOD(DestroyContextRejectsNullHandlePointer)
		{
			const auto result = AssetSuite::DestroyContext(nullptr);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidContext == result);
		}

		TEST_METHOD(DestroyContextRejectsNullHandle)
		{
			AssetSuite::ContextHandle context = nullptr;

			const auto result = AssetSuite::DestroyContext(&context);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidContext == result);
		}

		TEST_METHOD(DestroyContextReturnsInvalidContextAfterRepeatedDestroy)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::DestroyContext(&context));

			const auto result = AssetSuite::DestroyContext(&context);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidContext == result);
			Assert::IsNull(context);
		}

		TEST_METHOD(SetLoggingCallbackRejectsNullContext)
		{
			LogCapture capture = {};

			const auto result = AssetSuite::SetLoggingCallback(
				nullptr,
				&CaptureLogEvent,
				AssetSuite::LogLevel::Info,
				&capture);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidContext == result);
		}

		TEST_METHOD(SetLoggingCallbackRegistersCallbackAndPassesUserData)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			LogCapture capture = {};

			const auto result = AssetSuite::SetLoggingCallback(
				context,
				&CaptureLogEvent,
				AssetSuite::LogLevel::Info,
				&capture);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);

			AssetSuite::DispatchLogEvent(context, AssetSuite::LogLevel::Warning, "registered");

			Assert::AreEqual(1, capture.callCount);
			Assert::AreEqual(true, AssetSuite::LogLevel::Warning == capture.lastLevel);
			Assert::AreEqual("registered", capture.lastMessage);
			Assert::IsTrue(&capture == capture.lastUserData);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(SetLoggingCallbackAppliesMinimumLogLevelFiltering)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			LogCapture capture = {};
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::SetLoggingCallback(
				context,
				&CaptureLogEvent,
				AssetSuite::LogLevel::Warning,
				&capture));

			AssetSuite::DispatchLogEvent(context, AssetSuite::LogLevel::Info, "filtered");
			AssetSuite::DispatchLogEvent(context, AssetSuite::LogLevel::Warning, "warning");
			AssetSuite::DispatchLogEvent(context, AssetSuite::LogLevel::Error, "error");

			Assert::AreEqual(2, capture.callCount);
			Assert::AreEqual(true, AssetSuite::LogLevel::Error == capture.lastLevel);
			Assert::AreEqual("error", capture.lastMessage);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(SetLoggingCallbackReplacesCallbackState)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			LogCapture firstCapture = {};
			LogCapture secondCapture = {};
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::SetLoggingCallback(
				context,
				&CaptureLogEvent,
				AssetSuite::LogLevel::Info,
				&firstCapture));

			const auto result = AssetSuite::SetLoggingCallback(
				context,
				&CaptureLogEvent,
				AssetSuite::LogLevel::Error,
				&secondCapture);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);

			AssetSuite::DispatchLogEvent(context, AssetSuite::LogLevel::Warning, "filtered");
			AssetSuite::DispatchLogEvent(context, AssetSuite::LogLevel::Error, "replacement");

			Assert::AreEqual(0, firstCapture.callCount);
			Assert::AreEqual(1, secondCapture.callCount);
			Assert::AreEqual("replacement", secondCapture.lastMessage);
			Assert::IsTrue(&secondCapture == secondCapture.lastUserData);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(SetLoggingCallbackUnregistersNullCallback)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			LogCapture capture = {};
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::SetLoggingCallback(
				context,
				&CaptureLogEvent,
				AssetSuite::LogLevel::Trace,
				&capture));

			const auto result = AssetSuite::SetLoggingCallback(
				context,
				nullptr,
				AssetSuite::LogLevel::Trace,
				nullptr);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);

			AssetSuite::DispatchLogEvent(context, AssetSuite::LogLevel::Fatal, "unregistered");

			Assert::AreEqual(0, capture.callCount);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(RuntimeDiagnosticsAreContextScoped)
		{
			AssetSuite::ContextHandle firstContext = nullptr;
			AssetSuite::ContextHandle secondContext = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &firstContext));
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &secondContext));

			firstContext->Runtime().Diagnostics().Add(AssetSuite::ErrorCode::Undefined, "runtime smoke diagnostic");

			Assert::AreEqual(static_cast<size_t>(1), firstContext->Runtime().Diagnostics().Entries().size());
			Assert::AreEqual(static_cast<size_t>(0), secondContext->Runtime().Diagnostics().Entries().size());

			firstContext->Runtime().Diagnostics().Clear();
			Assert::AreEqual(static_cast<size_t>(0), firstContext->Runtime().Diagnostics().Entries().size());

			DestroyContextForCleanup(secondContext);
			DestroyContextForCleanup(firstContext);
		}

		TEST_METHOD(RuntimeCodecRegistryRejectsSentinelSlots)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));

			auto& registry = context->Runtime().CodecRegistry();
			AssetSuite::ImageDecoder* imageDecoder = registry.FindImageDecoder(AssetSuite::ImageDecoders::BMP);
			AssetSuite::MeshDecoder* meshDecoder = registry.FindMeshDecoder(AssetSuite::MeshDecoders::WAVEFRONT);

			Assert::IsNotNull(imageDecoder);
			Assert::IsNotNull(meshDecoder);
			Assert::IsNull(registry.FindImageDecoder(AssetSuite::ImageDecoders::Auto));
			Assert::IsNull(registry.FindImageDecoder(AssetSuite::ImageDecoders::MaxDecoders));
			Assert::IsNull(registry.FindMeshDecoder(AssetSuite::MeshDecoders::Auto));
			Assert::IsNull(registry.FindMeshDecoder(AssetSuite::MeshDecoders::MaxDecoders));
			Assert::IsFalse(registry.RegisterImageDecoder(AssetSuite::ImageDecoders::Auto, *imageDecoder));
			Assert::IsFalse(registry.RegisterImageDecoder(AssetSuite::ImageDecoders::MaxDecoders, *imageDecoder));
			Assert::IsFalse(registry.RegisterMeshDecoder(AssetSuite::MeshDecoders::Auto, *meshDecoder));
			Assert::IsFalse(registry.RegisterMeshDecoder(AssetSuite::MeshDecoders::MaxDecoders, *meshDecoder));

			DestroyContextForCleanup(context);
		}

	private:
		struct LogCapture
		{
			int callCount;
			AssetSuite::LogLevel lastLevel;
			const char* lastMessage;
			void* lastUserData;
		};

		static void AssertResultString(AssetSuite::Result result, const char* expected)
		{
			const char* actual = AssetSuite::GetResultString(result);

			Assert::IsNotNull(actual);
			Assert::AreEqual(expected, actual);
		}

		static void DestroyContextForCleanup(AssetSuite::ContextHandle& context)
		{
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::DestroyContext(&context));
			Assert::IsNull(context);
		}

		static void CaptureLogEvent(AssetSuite::LogLevel level, const char* message, void* userData)
		{
			auto* capture = static_cast<LogCapture*>(userData);
			++capture->callCount;
			capture->lastLevel = level;
			capture->lastMessage = message;
			capture->lastUserData = userData;
		}
	};

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
