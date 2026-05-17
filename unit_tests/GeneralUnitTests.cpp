#pragma warning (disable : 4251)
#include <CppUnitTest.h>

#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>
#include <Windows.h>

#include "../source/common/AssetSuite.h"
#include "../source/common/AssetSuiteContext.h"
#include "../source/runtime/AssetSuiteBlob.h"
#include "../source/runtime/AssetSuiteRuntime.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace
{
	std::filesystem::path GetLoadedModuleDirectory()
	{
		HMODULE module = nullptr;
		if (!GetModuleHandleExW(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCWSTR>(&GetLoadedModuleDirectory),
			&module))
		{
			return {};
		}

		wchar_t modulePath[MAX_PATH] = {};
		if (GetModuleFileNameW(module, modulePath, MAX_PATH) == 0)
		{
			return {};
		}

		return std::filesystem::path(modulePath).parent_path();
	}

	bool ContainsTestAssets(const std::filesystem::path& directory)
	{
		return !directory.empty() && std::filesystem::exists(directory / "test_file.xyz");
	}

	std::filesystem::path FindTestAssetDirectory()
	{
		const std::filesystem::path moduleDirectory = GetLoadedModuleDirectory();
		const std::filesystem::path currentDirectory = std::filesystem::current_path();
		const std::filesystem::path sourceDirectory = std::filesystem::path(__FILE__).parent_path();

		const std::filesystem::path candidates[] = {
			currentDirectory,
			moduleDirectory,
			currentDirectory / "test_images",
			moduleDirectory / "test_images",
			sourceDirectory.parent_path() / "test_images"
		};

		for (const auto& candidate : candidates)
		{
			if (ContainsTestAssets(candidate))
			{
				return candidate;
			}
		}

		for (std::filesystem::path cursor = currentDirectory; !cursor.empty(); cursor = cursor.parent_path())
		{
			if (ContainsTestAssets(cursor))
			{
				return cursor;
			}

			if (ContainsTestAssets(cursor / "test_images"))
			{
				return cursor / "test_images";
			}

			if (cursor == cursor.parent_path())
			{
				break;
			}
		}

		return currentDirectory;
	}
}

namespace GeneralUnitTests
{
	TEST_MODULE_INITIALIZE(ModuleInitialize)
	{
		std::filesystem::current_path(FindTestAssetDirectory());
	}

	TEST_CLASS(RuntimeBlobTests)
	{
	public:
		TEST_METHOD(BlobStoresRawBytesAndMetadata)
		{
			std::vector<uint8_t> bytes = { 0x01, 0x02, 0x03, 0x04 };
			const auto metadata = AssetSuite::Internal::MakeBlobSourceMetadata("textures/albedo.png");

			AssetSuite::Internal::Blob blob(std::move(bytes), metadata);
			const auto desc = blob.Describe();

			Assert::AreEqual(static_cast<uint64_t>(4), blob.ByteSize());
			Assert::IsNotNull(blob.Data());
			Assert::AreEqual(static_cast<uint8_t>(0x01), blob.Data()[0]);
			Assert::AreEqual(true, blob.SourceMetadata().sourcePath == "textures/albedo.png");
			Assert::AreEqual(true, blob.SourceMetadata().extension == ".png");
			Assert::AreEqual(true, AssetSuite::AssetFormat::PNG == blob.SourceMetadata().format);
			Assert::AreEqual(static_cast<uint32_t>(sizeof(AssetSuite::BlobDesc)), desc.structSize);
			Assert::AreEqual(static_cast<uint64_t>(4), desc.byteSize);
			Assert::AreEqual(true, AssetSuite::AssetFormat::PNG == desc.format);
			Assert::AreEqual(static_cast<uint32_t>(0), desc.flags);
		}

		TEST_METHOD(BlobMetadataUsesUnknownFormatForUnsupportedExtension)
		{
			const auto metadata = AssetSuite::Internal::MakeBlobSourceMetadata("data/source.asset");

			Assert::AreEqual(true, metadata.sourcePath == "data/source.asset");
			Assert::AreEqual(true, metadata.extension == ".asset");
			Assert::AreEqual(true, AssetSuite::AssetFormat::Unknown == metadata.format);
		}

		TEST_METHOD(BlobMetadataNormalizesExtensionCase)
		{
			const auto pngMetadata = AssetSuite::Internal::MakeBlobSourceMetadata("textures/ALBEDO.PNG");
			const auto objMetadata = AssetSuite::Internal::MakeBlobSourceMetadata("meshes/CUBE.OBJ");

			Assert::AreEqual(true, pngMetadata.extension == ".PNG");
			Assert::AreEqual(true, AssetSuite::AssetFormat::PNG == pngMetadata.format);
			Assert::AreEqual(true, objMetadata.extension == ".OBJ");
			Assert::AreEqual(true, AssetSuite::AssetFormat::WavefrontObj == objMetadata.format);
		}
	};

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
			AssertResultString(AssetSuite::Result::ErrorInvalidHandle, "Error: invalid handle");
			AssertResultString(AssetSuite::Result::ErrorIoFailure, "Error: IO failure");
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

		TEST_METHOD(LoadFileCreatesRuntimeOwnedBlob)
		{
			AssetSuite::ContextHandle context = nullptr;
			AssetSuite::BlobHandle blob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));

			const auto result = AssetSuite::LoadFile(context, "test_file.xyz", &blob);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);
			Assert::IsNotNull(blob);
			Assert::AreEqual(static_cast<size_t>(1), context->Runtime().BlobStorage().LiveCount());
			const auto* storedBlob = context->Runtime().BlobStorage().Get(blob);
			Assert::IsNotNull(storedBlob);
			Assert::AreEqual(static_cast<uint64_t>(102), storedBlob->ByteSize());
			Assert::AreEqual(true, AssetSuite::AssetFormat::Unknown == storedBlob->SourceMetadata().format);

			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::ReleaseBlob(context, &blob));
			DestroyContextForCleanup(context);
		}

		TEST_METHOD(LoadFileRejectsNullContext)
		{
			AssetSuite::BlobHandle blob = nullptr;

			const auto result = AssetSuite::LoadFile(nullptr, "test_file.xyz", &blob);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidContext == result);
			Assert::IsNull(blob);
		}

		TEST_METHOD(LoadFileRejectsInvalidArguments)
		{
			AssetSuite::ContextHandle context = nullptr;
			AssetSuite::BlobHandle blob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidArgument == AssetSuite::LoadFile(context, nullptr, &blob));
			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidArgument == AssetSuite::LoadFile(context, "test_file.xyz", nullptr));

			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::LoadFile(context, "test_file.xyz", &blob));
			AssetSuite::BlobHandle originalBlob = blob;
			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidArgument == AssetSuite::LoadFile(context, "test_file.xyz", &blob));
			Assert::IsTrue(originalBlob == blob);

			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::ReleaseBlob(context, &blob));
			DestroyContextForCleanup(context);
		}

		TEST_METHOD(LoadFileReturnsFileNotFoundForMissingFile)
		{
			AssetSuite::ContextHandle context = nullptr;
			AssetSuite::BlobHandle blob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));

			const auto result = AssetSuite::LoadFile(context, "missing_blob_source.bin", &blob);

			Assert::AreEqual(true, AssetSuite::Result::ErrorFileNotFound == result);
			Assert::IsNull(blob);
			Assert::AreEqual(static_cast<size_t>(0), context->Runtime().BlobStorage().LiveCount());

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(LoadFileReturnsIoFailureForExistingNonFilePath)
		{
			AssetSuite::ContextHandle context = nullptr;
			AssetSuite::BlobHandle blob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));

			const auto result = AssetSuite::LoadFile(context, std::filesystem::current_path().string().c_str(), &blob);

			Assert::AreEqual(true, AssetSuite::Result::ErrorIoFailure == result);
			Assert::IsNull(blob);
			Assert::AreEqual(static_cast<size_t>(0), context->Runtime().BlobStorage().LiveCount());

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(RuntimeFileLoaderPreservesOutputOnIoFailure)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			std::vector<uint8_t> bytes = { 0xAB, 0xCD };

			const auto result = context->Runtime().FileLoader().LoadToMemory(std::filesystem::current_path(), true, bytes);

			Assert::AreEqual(true, AssetSuite::ErrorCode::IoFailure == result);
			Assert::AreEqual(static_cast<size_t>(2), bytes.size());
			Assert::AreEqual(static_cast<uint8_t>(0xAB), bytes[0]);
			Assert::AreEqual(static_cast<uint8_t>(0xCD), bytes[1]);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(RuntimeFileLoaderAddsTerminatorForTextLoads)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			std::vector<uint8_t> textBytes;
			std::vector<uint8_t> binaryBytes;

			const auto textResult = context->Runtime().FileLoader().LoadToMemory("test_mesh.obj", false, textBytes);
			const auto binaryResult = context->Runtime().FileLoader().LoadToMemory("test_mesh.obj", true, binaryBytes);

			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == textResult);
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == binaryResult);
			Assert::AreEqual(binaryBytes.size() + 1, textBytes.size());
			Assert::AreEqual(static_cast<uint8_t>('\0'), textBytes.back());

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(ReleaseBlobClearsHandleAndInvalidatesStorage)
		{
			AssetSuite::ContextHandle context = nullptr;
			AssetSuite::BlobHandle blob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::LoadFile(context, "test_file.xyz", &blob));

			const auto result = AssetSuite::ReleaseBlob(context, &blob);

			Assert::AreEqual(true, AssetSuite::Result::Success == result);
			Assert::IsNull(blob);
			Assert::AreEqual(static_cast<size_t>(0), context->Runtime().BlobStorage().LiveCount());

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(ReleaseBlobRejectsInvalidInputs)
		{
			AssetSuite::ContextHandle context = nullptr;
			AssetSuite::BlobHandle blob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidContext == AssetSuite::ReleaseBlob(nullptr, &blob));
			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidHandle == AssetSuite::ReleaseBlob(context, nullptr));
			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidHandle == AssetSuite::ReleaseBlob(context, &blob));

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(ReleaseBlobRejectsDoubleReleaseAndStaleCopiedHandle)
		{
			AssetSuite::ContextHandle context = nullptr;
			AssetSuite::BlobHandle blob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::LoadFile(context, "test_file.xyz", &blob));
			AssetSuite::BlobHandle copiedBlob = blob;

			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::ReleaseBlob(context, &blob));

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidHandle == AssetSuite::ReleaseBlob(context, &blob));
			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidHandle == AssetSuite::ReleaseBlob(context, &copiedBlob));
			Assert::IsNotNull(copiedBlob);

			DestroyContextForCleanup(context);
		}

		TEST_METHOD(ReleaseBlobRejectsWrongContext)
		{
			AssetSuite::ContextHandle firstContext = nullptr;
			AssetSuite::ContextHandle secondContext = nullptr;
			AssetSuite::BlobHandle firstBlob = nullptr;
			AssetSuite::BlobHandle secondBlob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &firstContext));
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &secondContext));
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::LoadFile(firstContext, "test_file.xyz", &firstBlob));
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::LoadFile(secondContext, "test_file.xyz", &secondBlob));
			Assert::IsFalse(firstBlob == secondBlob);

			const auto result = AssetSuite::ReleaseBlob(secondContext, &firstBlob);

			Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidHandle == result);
			Assert::IsNotNull(firstBlob);
			Assert::AreEqual(static_cast<size_t>(1), firstContext->Runtime().BlobStorage().LiveCount());
			Assert::AreEqual(static_cast<size_t>(1), secondContext->Runtime().BlobStorage().LiveCount());

			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::ReleaseBlob(secondContext, &secondBlob));
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::ReleaseBlob(firstContext, &firstBlob));
			DestroyContextForCleanup(secondContext);
			DestroyContextForCleanup(firstContext);
		}

		TEST_METHOD(BlobStorageReusesReleasedSlotsAndRejectsOldGenerations)
		{
			AssetSuite::ContextHandle context = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::CreateContext(nullptr, &context));

			AssetSuite::BlobHandle blob = nullptr;
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::LoadFile(context, "test_file.xyz", &blob));
			AssetSuite::BlobHandle staleBlob = blob;
			Assert::AreEqual(static_cast<size_t>(1), context->Runtime().BlobStorage().SlotCapacity());
			Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::ReleaseBlob(context, &blob));

			for (int iteration = 0; iteration < 8; ++iteration)
			{
				Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::LoadFile(context, "test_file.xyz", &blob));
				Assert::AreEqual(static_cast<size_t>(1), context->Runtime().BlobStorage().LiveCount());
				Assert::AreEqual(static_cast<size_t>(1), context->Runtime().BlobStorage().SlotCapacity());
				Assert::AreEqual(true, AssetSuite::Result::ErrorInvalidHandle == AssetSuite::ReleaseBlob(context, &staleBlob));
				Assert::AreEqual(true, AssetSuite::Result::Success == AssetSuite::ReleaseBlob(context, &blob));
			}

			Assert::AreEqual(static_cast<size_t>(0), context->Runtime().BlobStorage().LiveCount());
			Assert::AreEqual(static_cast<size_t>(1), context->Runtime().BlobStorage().SlotCapacity());
			Assert::IsNotNull(staleBlob);

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

		TEST_METHOD(ImageOpeningNonExistingFile)
		{
			AssetSuite::Manager manager;

			auto error = manager.ImageLoad("non-existing-image.bmp");

			Assert::AreEqual(true, AssetSuite::ErrorCode::NonExistingFile == error);
		}

		TEST_METHOD(ImageFailedLoadClearsPreviousRawBuffer)
		{
			AssetSuite::Manager manager;

			auto error = manager.ImageLoad("test_file.xyz");
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			error = manager.ImageLoad("non-existing-image.bmp");
			Assert::AreEqual(true, AssetSuite::ErrorCode::NonExistingFile == error);

			error = manager.ImageDecode(AssetSuite::ImageDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::RawBufferIsEmpty == error);
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

		TEST_METHOD(FailedMeshLoadClearsPreviousRawBuffer)
		{
			AssetSuite::Manager manager;

			auto error = manager.MeshLoad("test_mesh.obj");
			Assert::AreEqual(true, AssetSuite::ErrorCode::OK == error);

			error = manager.MeshLoad("non-existing-mesh.obj");
			Assert::AreEqual(true, AssetSuite::ErrorCode::NonExistingFile == error);

			error = manager.MeshDecode(AssetSuite::MeshDecoders::Auto);
			Assert::AreEqual(true, AssetSuite::ErrorCode::RawBufferIsEmpty == error);
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
