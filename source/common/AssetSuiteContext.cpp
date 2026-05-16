#include "AssetSuiteContext.h"

#include "../runtime/AssetSuiteRuntime.h"

#include "AssetSuite.h"

#include <new>

AssetSuite::AssetSuiteContext_t::AssetSuiteContext_t(const ContextDesc& desc)
	: runtime(std::make_unique<Internal::RuntimeContext>(desc))
{
}

AssetSuite::AssetSuiteContext_t::~AssetSuiteContext_t() = default;

AssetSuite::Internal::RuntimeContext& AssetSuite::AssetSuiteContext_t::Runtime()
{
	return *runtime;
}

const AssetSuite::Internal::RuntimeContext& AssetSuite::AssetSuiteContext_t::Runtime() const
{
	return *runtime;
}

AssetSuite::Result AssetSuite::Internal::CreateContextHandle(
	const ContextDesc& desc,
	ContextHandle* outContext) noexcept
{
	try
	{
		std::unique_ptr<AssetSuiteContext_t> context = std::make_unique<AssetSuiteContext_t>(desc);
		*outContext = context.release();
	}
	catch (const std::bad_alloc&)
	{
		return Result::ErrorOutOfMemory;
	}
	catch (...)
	{
		return Result::ErrorUnknown;
	}

	return Result::Success;
}

AssetSuite::Result AssetSuite::Internal::DestroyContextHandle(ContextHandle* context) noexcept
{
	if (!context || !*context)
	{
		return Result::ErrorInvalidContext;
	}

	try
	{
		delete *context;
		*context = nullptr;
	}
	catch (...)
	{
		return Result::ErrorUnknown;
	}

	return Result::Success;
}

void AssetSuite::DispatchLogEvent(ContextHandle context, LogLevel level, const char* message)
{
	if (!context)
	{
		return;
	}

	context->Runtime().DispatchLogEvent(level, message);
}

AssetSuite::Result AssetSuite::CaptureRuntimeSmokeStatus(ContextHandle context, RuntimeSmokeStatus* outStatus)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!outStatus)
	{
		return Result::ErrorInvalidArgument;
	}

	auto& runtime = context->Runtime();
	auto& codecRegistry = runtime.CodecRegistry();
	outStatus->descriptorStructSize = runtime.Descriptor().structSize;
	outStatus->descriptorFlags = runtime.Descriptor().flags;
	outStatus->diagnosticsEntryCount = static_cast<uint32_t>(runtime.Diagnostics().Entries().size());
	outStatus->hasBmpDecoder = codecRegistry.FindImageDecoder(ImageDecoders::BMP) != nullptr;
	outStatus->hasPngDecoder = codecRegistry.FindImageDecoder(ImageDecoders::PNG) != nullptr;
	outStatus->hasWavefrontDecoder = codecRegistry.FindMeshDecoder(MeshDecoders::WAVEFRONT) != nullptr;
	outStatus->rejectsImageSentinels =
		codecRegistry.FindImageDecoder(ImageDecoders::Auto) == nullptr &&
		codecRegistry.FindImageDecoder(ImageDecoders::MaxDecoders) == nullptr;
	outStatus->rejectsMeshSentinels =
		codecRegistry.FindMeshDecoder(MeshDecoders::Auto) == nullptr &&
		codecRegistry.FindMeshDecoder(MeshDecoders::MaxDecoders) == nullptr;

	return Result::Success;
}

AssetSuite::Result AssetSuite::AddRuntimeSmokeDiagnostic(ContextHandle context)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	context->Runtime().Diagnostics().Add(ErrorCode::Undefined, "runtime smoke diagnostic");
	return Result::Success;
}

AssetSuite::Result AssetSuite::ClearRuntimeSmokeDiagnostics(ContextHandle context)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	context->Runtime().Diagnostics().Clear();
	return Result::Success;
}
