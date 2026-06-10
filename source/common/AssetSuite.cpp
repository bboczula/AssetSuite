#pragma warning (disable : 4251)

#include "AssetSuite.h"
#include "AssetSuiteContext.h"

#include "../runtime/AssetSuiteRuntime.h"
#include "../runtime/AssetSuiteRuntimeDiagnostics.h"
#include "../runtime/AssetSuiteRuntimeState.h"
#include "../wavefront/ModelLoader.h"
#include "../bmp/BmpDecoder.h"
#include "../png/PngDecoder.h"
#include "../ppm/PpmEncoder.h"
#include "../bypass/BypassEncoder.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <new>

namespace
{
	constexpr AssetSuite::Version ASSET_SUITE_VERSION = { 2, 0, 0, 0 };
	constexpr AssetSuite::ContextDesc DEFAULT_CONTEXT_DESC = { sizeof(AssetSuite::ContextDesc), 0 };

	AssetSuite::Result NormalizeContextDesc(const AssetSuite::ContextDesc* desc, AssetSuite::ContextDesc& normalizedDesc)
	{
		if (!desc)
		{
			normalizedDesc = DEFAULT_CONTEXT_DESC;
			return AssetSuite::Result::Success;
		}

		if (desc->structSize != sizeof(AssetSuite::ContextDesc))
		{
			return AssetSuite::Result::ErrorInvalidArgument;
		}

		if (desc->flags != 0)
		{
			return AssetSuite::Result::ErrorInvalidArgument;
		}

		normalizedDesc = *desc;
		return AssetSuite::Result::Success;
	}

	AssetSuite::Result MapFileLoadResult(AssetSuite::ErrorCode error)
	{
		switch (error)
		{
		case AssetSuite::ErrorCode::OK:
			return AssetSuite::Result::Success;
		case AssetSuite::ErrorCode::NonExistingFile:
			return AssetSuite::Result::ErrorFileNotFound;
		case AssetSuite::ErrorCode::IoFailure:
			return AssetSuite::Result::ErrorIoFailure;
		default:
			return AssetSuite::Result::ErrorUnknown;
		}
	}

	AssetSuite::ErrorCode LoadRuntimeFileToMemory(
		AssetSuite::Internal::RuntimeContext& runtime,
		const std::filesystem::path& filePath,
		bool isBinary,
		std::vector<uint8_t>& output)
	{
		return runtime.FileLoader().LoadToMemory(filePath, isBinary, output);
	}

	AssetSuite::ErrorCode LoadRuntimeFileToMemory(
		AssetSuite::Internal::RuntimeState& runtimeState,
		const std::filesystem::path& filePath,
		bool isBinary,
		std::vector<uint8_t>& output)
	{
		return runtimeState.fileLoader.LoadToMemory(filePath, isBinary, output);
	}
}

AssetSuite::Result AssetSuite::GetVersion(Version* outVersion)
{
	if (!outVersion)
	{
		return Result::ErrorInvalidArgument;
	}

	*outVersion = ASSET_SUITE_VERSION;
	return Result::Success;
}

const char* AssetSuite::GetResultString(Result result)
{
	switch (result)
	{
	case Result::Success:
		return "Success";
	case Result::WarningUnsupportedChunk:
		return "Warning: unsupported chunk";
	case Result::ErrorInvalidArgument:
		return "Error: invalid argument";
	case Result::ErrorUnsupportedFormat:
		return "Error: unsupported format";
	case Result::ErrorFileNotFound:
		return "Error: file not found";
	case Result::ErrorDecodeFailed:
		return "Error: decode failed";
	case Result::ErrorOutputBufferTooSmall:
		return "Error: output buffer too small";
	case Result::ErrorOutOfMemory:
		return "Error: out of memory";
	case Result::ErrorInvalidContext:
		return "Error: invalid context";
	case Result::ErrorInvalidHandle:
		return "Error: invalid handle";
	case Result::ErrorIoFailure:
		return "Error: IO failure";
	case Result::ErrorMalformedData:
		return "Error: malformed data";
	case Result::ErrorUnknown:
		return "Error: unknown";
	default:
		return "Unknown result";
	}
}

AssetSuite::Result AssetSuite::CreateContext(const ContextDesc* desc, ContextHandle* outContext)
{
	if (!outContext)
	{
		return Result::ErrorInvalidArgument;
	}

	if (*outContext)
	{
		return Result::ErrorInvalidArgument;
	}

	ContextDesc normalizedDesc = DEFAULT_CONTEXT_DESC;
	const Result validationResult = NormalizeContextDesc(desc, normalizedDesc);
	if (validationResult != Result::Success)
	{
		return validationResult;
	}

	return Internal::CreateContextHandle(normalizedDesc, outContext);
}

AssetSuite::Result AssetSuite::DestroyContext(ContextHandle* context)
{
	return Internal::DestroyContextHandle(context);
}

AssetSuite::Result AssetSuite::SetLoggingCallback(
	ContextHandle context,
	LoggingCallback callback,
	LogLevel minLevel,
	void* userData)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	context->Runtime().SetLoggingCallback(callback, minLevel, userData);
	return Result::Success;
}

AssetSuite::Result AssetSuite::LoadFile(ContextHandle context, const char* filePath, BlobHandle* outBlob)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!filePath || !outBlob || *outBlob)
	{
		return Result::ErrorInvalidArgument;
	}

	std::vector<uint8_t> rawBytes;
	const ErrorCode loadResult = LoadRuntimeFileToMemory(context->Runtime(), filePath, true, rawBytes);
	const Result mappedResult = MapFileLoadResult(loadResult);
	if (mappedResult != Result::Success)
	{
		context->Runtime().EmitDiagnostic(
			loadResult,
			LogLevel::Error,
			Internal::Diagnostics::BlobLoadFailed);
		return mappedResult;
	}

	try
	{
		Internal::Blob blob(std::move(rawBytes), Internal::MakeBlobSourceMetadata(filePath));
		*outBlob = context->Runtime().BlobStorage().Create(std::move(blob));
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

AssetSuite::Result AssetSuite::DecodeImage(ContextHandle context, BlobHandle blob, ImageHandle* outImage)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!blob)
	{
		return Result::ErrorInvalidHandle;
	}

	if (!outImage || *outImage)
	{
		return Result::ErrorInvalidArgument;
	}

	const Internal::Blob* storedBlob = context->Runtime().BlobStorage().Get(blob);
	if (!storedBlob)
	{
		return Result::ErrorInvalidHandle;
	}

	return context->Runtime().DecodeImageBlob(*storedBlob, outImage);
}

AssetSuite::Result AssetSuite::DecodeImageFromFile(ContextHandle context, const char* filePath, ImageHandle* outImage)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!filePath || !outImage || *outImage)
	{
		return Result::ErrorInvalidArgument;
	}

	std::vector<uint8_t> rawBytes;
	const ErrorCode loadResult = LoadRuntimeFileToMemory(context->Runtime(), filePath, true, rawBytes);
	const Result mappedResult = MapFileLoadResult(loadResult);
	if (mappedResult != Result::Success)
	{
		context->Runtime().EmitDiagnostic(
			loadResult,
			LogLevel::Error,
			Internal::Diagnostics::ImageLoadFailed);
		return mappedResult;
	}

	try
	{
		Internal::Blob blob(std::move(rawBytes), Internal::MakeBlobSourceMetadata(filePath));
		// DecodeImageBlob owns probe/decode diagnostics so this wrapper does not duplicate them.
		return context->Runtime().DecodeImageBlob(blob, outImage);
	}
	catch (const std::bad_alloc&)
	{
		return Result::ErrorOutOfMemory;
	}
	catch (...)
	{
		return Result::ErrorUnknown;
	}
}

AssetSuite::Result AssetSuite::DecodeMesh(ContextHandle context, BlobHandle blob, MeshHandle* outMesh)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!blob)
	{
		return Result::ErrorInvalidHandle;
	}

	if (!outMesh || *outMesh)
	{
		return Result::ErrorInvalidArgument;
	}

	const Internal::Blob* storedBlob = context->Runtime().BlobStorage().Get(blob);
	if (!storedBlob)
	{
		return Result::ErrorInvalidHandle;
	}

	return context->Runtime().DecodeMeshBlob(*storedBlob, outMesh);
}

AssetSuite::Result AssetSuite::DecodeMeshFromFile(ContextHandle context, const char* filePath, MeshHandle* outMesh)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!filePath || !outMesh || *outMesh)
	{
		return Result::ErrorInvalidArgument;
	}

	std::vector<uint8_t> rawBytes;
	const ErrorCode loadResult = LoadRuntimeFileToMemory(context->Runtime(), filePath, true, rawBytes);
	const Result mappedResult = MapFileLoadResult(loadResult);
	if (mappedResult != Result::Success)
	{
		context->Runtime().EmitDiagnostic(
			loadResult,
			LogLevel::Error,
			Internal::Diagnostics::MeshLoadFailed);
		return mappedResult;
	}

	try
	{
		Internal::Blob blob(std::move(rawBytes), Internal::MakeBlobSourceMetadata(filePath));
		// DecodeMeshBlob owns probe/decode diagnostics so this wrapper does not duplicate them.
		return context->Runtime().DecodeMeshBlob(blob, outMesh);
	}
	catch (const std::bad_alloc&)
	{
		return Result::ErrorOutOfMemory;
	}
	catch (...)
	{
		return Result::ErrorUnknown;
	}
}

AssetSuite::Result AssetSuite::GetImageDesc(ContextHandle context, ImageHandle image, ImageDesc* outDesc)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!outDesc)
	{
		return Result::ErrorInvalidArgument;
	}

	const AssetSuiteImage_t* storedImage = context->Runtime().ImageStorage().Get(image);
	if (!storedImage)
	{
		return Result::ErrorInvalidHandle;
	}

	*outDesc = storedImage->desc;
	return Result::Success;
}

AssetSuite::Result AssetSuite::GetMeshDesc(ContextHandle context, MeshHandle mesh, MeshDesc* outDesc)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!outDesc)
	{
		return Result::ErrorInvalidArgument;
	}

	const AssetSuiteMesh_t* storedMesh = context->Runtime().MeshStorage().Get(mesh);
	if (!storedMesh)
	{
		return Result::ErrorInvalidHandle;
	}

	*outDesc = storedMesh->desc;
	return Result::Success;
}

AssetSuite::Result AssetSuite::ReleaseImage(ContextHandle context, ImageHandle* image)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	return context->Runtime().ImageStorage().Release(image);
}

AssetSuite::Result AssetSuite::ReleaseMesh(ContextHandle context, MeshHandle* mesh)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	return context->Runtime().MeshStorage().Release(mesh);
}

AssetSuite::Result AssetSuite::ReleaseBlob(ContextHandle context, BlobHandle* blob)
{
	if (!context)
	{
		return Result::ErrorInvalidContext;
	}

	if (!blob || !*blob)
	{
		return Result::ErrorInvalidHandle;
	}

	return context->Runtime().BlobStorage().Release(blob);
}

AssetSuite::Manager::Manager()
	: runtimeState(new Internal::RuntimeState())
	, ownsRuntimeState(true)
{
}

AssetSuite::Manager::Manager(Internal::RuntimeState& runtimeState)
	: runtimeState(&runtimeState)
	, ownsRuntimeState(false)
{
}

AssetSuite::Manager::~Manager()
{
	if (ownsRuntimeState)
	{
		delete runtimeState;
	}
}

AssetSuite::Internal::RuntimeState& AssetSuite::Manager::State()
{
	return *runtimeState;
}

const AssetSuite::Internal::RuntimeState& AssetSuite::Manager::State() const
{
	return *runtimeState;
}

AssetSuite::ErrorCode AssetSuite::Manager::ImageLoadAndDecode(const char* filePathAndName, ImageDecoders decoder)
{
      auto loadResult = ImageLoad(filePathAndName);
      if (loadResult != ErrorCode::OK)
      {
            return loadResult;
      }

      auto decodeResult = ImageDecode(ImageDecoders::Auto);
      if (decodeResult != ErrorCode::OK)
      {
            return decodeResult;
      }

      return decodeResult;
}

AssetSuite::ErrorCode AssetSuite::Manager::ImageLoad(const char* filePathAndName)
{
      auto& state = State();
      state.fileInfo.fullName = filePathAndName;
      state.fileInfo.extension = state.fileInfo.fullName.extension();
      return LoadFileToMemory(filePathAndName);
}

AssetSuite::ErrorCode AssetSuite::Manager::ImageDecode(ImageDecoders decoder)
{
      auto& state = State();
      if (state.rawBuffer.empty())
      {
            return ErrorCode::RawBufferIsEmpty;
      }

      if (decoder == ImageDecoders::Auto)
      {
            decoder = state.codecRegistry.ProbeImageDecoder(
                  state.fileInfo.extension,
                  state.rawBuffer.data(),
                  state.rawBuffer.size());
            if (decoder == ImageDecoders::Auto)
            {
                  state.diagnostics.Add(ErrorCode::FileTypeNotSupported, "Image file type is not supported.");
                  return ErrorCode::FileTypeNotSupported;
            }
      }

      ImageDecoder* imageDecoder = state.codecRegistry.FindImageDecoder(decoder);
      if (!imageDecoder)
      {
            state.diagnostics.Add(ErrorCode::FileTypeNotSupported, "Image decoder is not registered.");
            return ErrorCode::FileTypeNotSupported;
      }

      ImageDescriptor descriptor;
      auto error = imageDecoder->Decode(state.decodedBuffer, state.rawBuffer.data(), descriptor);

      state.imageInfo.width = descriptor.width;
      state.imageInfo.height = descriptor.height;
      state.imageInfo.format = descriptor.format;

      return error ? ErrorCode::OK : ErrorCode::Undefined;
}

AssetSuite::ErrorCode AssetSuite::Manager::ImageGet(OutputFormat format, std::vector<BYTE>& output, ImageDescriptor& descriptor)
{
      const auto& state = State();
      if (state.decodedBuffer.empty())
      {
            return ErrorCode::DecodedBufferIsEmpty;
      }

      output = state.decodedBuffer;
      descriptor.width = state.imageInfo.width;
      descriptor.height = state.imageInfo.height;
      descriptor.format = state.imageInfo.format;

      return ErrorCode::OK;
}

void AssetSuite::Manager::StoreMeshToFile(const std::string& filePathAndName, BYTE* buffer, const MeshDescriptor& imageDescriptor)
{
}

AssetSuite::ErrorCode AssetSuite::Manager::MeshLoadAndDecode(const char* filePathAndName, MeshDecoders decoder)
{
      auto loadResult = MeshLoad(filePathAndName);
      if (loadResult != ErrorCode::OK)
      {
            return loadResult;
      }

      auto decodeResult = MeshDecode(decoder);
      if (decodeResult != ErrorCode::OK)
      {
            return decodeResult;
      }

      return decodeResult;
}

AssetSuite::ErrorCode AssetSuite::Manager::MeshLoad(const char* filePathAndName)
{
      auto& state = State();
      state.fileInfo.fullName = filePathAndName;
      state.fileInfo.extension = state.fileInfo.fullName.extension();
      return LoadFileToMemory(filePathAndName, false);
}

AssetSuite::ErrorCode AssetSuite::Manager::MeshDecode(MeshDecoders decoder)
{
      auto& state = State();
      if (state.rawBuffer.empty())
      {
            return ErrorCode::RawBufferIsEmpty;
      }

      if (decoder == MeshDecoders::Auto)
      {
            decoder = state.codecRegistry.ProbeMeshDecoder(
                  state.fileInfo.extension,
                  state.rawBuffer.data(),
                  state.rawBuffer.size());
            if (decoder == MeshDecoders::Auto)
            {
                  state.diagnostics.Add(ErrorCode::FileTypeNotSupported, "Mesh file type is not supported.");
                  return ErrorCode::FileTypeNotSupported;
            }
      }

      MeshDecoder* meshDecoder = state.codecRegistry.FindMeshDecoder(decoder);
      if (!meshDecoder)
      {
            state.diagnostics.Add(ErrorCode::FileTypeNotSupported, "Mesh decoder is not registered.");
            return ErrorCode::FileTypeNotSupported;
      }

      std::vector<BYTE> output;
      MeshDescriptor descriptor;
      auto error = meshDecoder->Decode(output, state.rawBuffer.data(), descriptor);
      return error ? ErrorCode::OK : ErrorCode::Undefined;
}

AssetSuite::ErrorCode AssetSuite::Manager::MeshGet(const char* meshName, MeshOutputFormat format, std::vector<FLOAT>& output, MeshDescriptor& descriptor)
{
      auto& state = State();
      // Fetch the group data
      auto& modelLoader = *state.codecs.modelLoader;
      auto groupOffset = modelLoader.GetGroupOffset(meshName);
      auto groupSize = modelLoader.GetGroupSize(meshName);
      state.meshInfo.numOfVertices = groupSize;
      state.meshInfo.numOfIndices = groupSize;
      descriptor.numOfVertices = state.meshInfo.numOfVertices;
      descriptor.numOfIndices = state.meshInfo.numOfIndices;

      if (format == MeshOutputFormat::POSITION)
      {
            // Each face has three vertices, each vertex has four elements
            output.resize(groupSize * 3 * 4);
            for (UINT i = 0; i < groupSize; i++)
            {
                  auto face = modelLoader.GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto vertex = modelLoader.GetVertex(face.vertexIndex[j]);
                        output[i * 3 * 4 + j * 4 + 0] = vertex.x;
                        output[i * 3 * 4 + j * 4 + 1] = vertex.y;
                        output[i * 3 * 4 + j * 4 + 2] = vertex.z;
                        output[i * 3 * 4 + j * 4 + 3] = 1.0f;
                  }
            }
      }
      else if (format == MeshOutputFormat::NORMAL)
      {
            output.resize(groupSize * 3 * 4);
            for (UINT i = 0; i < groupSize; i++)
            {
                  auto face = modelLoader.GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto normal = modelLoader.GetNormal(face.normalIndex[j]);
                        output[i * 3 * 4 + j * 4 + 0] = normal.x;
                        output[i * 3 * 4 + j * 4 + 1] = normal.y;
                        output[i * 3 * 4 + j * 4 + 2] = normal.z;
                        output[i * 3 * 4 + j * 4 + 3] = 0.0f;
                  }
            }
      }
      else if (format == MeshOutputFormat::TANGENT)
      {
            output.resize(groupSize * 3 * 4);
            for (UINT i = 0; i < groupSize; i++)
            {
                  auto face = modelLoader.GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto tangent = modelLoader.GetTangent(face.normalIndex[j]);
                        output[i * 3 * 4 + j * 4 + 0] = tangent.x;
                        output[i * 3 * 4 + j * 4 + 1] = tangent.y;
                        output[i * 3 * 4 + j * 4 + 2] = tangent.z;
                        output[i * 3 * 4 + j * 4 + 3] = 0.0f;
                  }
            }
      }
      else if (format == MeshOutputFormat::TEXCOORD)
      {
            output.resize(groupSize * 3 * 2);
            for (UINT i = 0; i < groupSize; i++)
            {
                  auto face = modelLoader.GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto texCoord = modelLoader.GetTextureCoord(face.textureIndex[j]);
                        output[i * 3 * 2 + j * 2 + 0] = texCoord.x;
                        output[i * 3 * 2 + j * 2 + 1] = texCoord.y;
                  }
            }
      }

      return ErrorCode();
}

AssetSuite::ErrorCode AssetSuite::Manager::DumpRawBuffer()
{
      ImageDescriptor descriptor;
      DumpBuffer("rawBuffer.txt", State().rawBuffer, descriptor);
      return ErrorCode::OK;
}

AssetSuite::ErrorCode AssetSuite::Manager::DumpDecodedBuffer()
{
      ImageDescriptor descriptor;
      DumpBuffer("decodedBuffer.txt", State().decodedBuffer, descriptor);
      return ErrorCode::OK;
}

void AssetSuite::Manager::StoreImageToFile(const std::string& filePathAndName, const std::vector<BYTE>& buffer, const ImageDescriptor& imageDescriptor)
{
      auto& state = State();
      state.rawBuffer = state.codecs.ppmEncoder->Encode(buffer, imageDescriptor);
      StoreMemoryToFile(state.rawBuffer, filePathAndName);
}

AssetSuite::ErrorCode AssetSuite::Manager::LoadFileToMemory(const std::string& fileName, bool isBinary)
{
      auto& state = State();
      const ErrorCode result = ::LoadRuntimeFileToMemory(state, fileName, isBinary, state.rawBuffer);
      if (result != ErrorCode::OK)
      {
            state.rawBuffer.clear();
            state.diagnostics.Add(result, "Failed to load file into runtime memory.");
      }

      return result;
}

void AssetSuite::Manager::StoreMemoryToFile(const std::vector<BYTE>& buffer, const std::string& fileName)
{
      // Open file for writing, it discards its previous contets
      std::ofstream file(fileName.c_str(), std::ios::out | std::ios::binary);
      file.write((char*)buffer.data(), buffer.size());
      file.close();
}

void AssetSuite::Manager::DumpByteVectorToCpp(const std::vector<BYTE>& byteVector)
{
      for (auto it = byteVector.begin(); it != byteVector.end(); it++)
      {
            std::cout << std::hex << (UINT)*it << std::endl;
      }
}

void AssetSuite::Manager::DumpBuffer(const std::string& fileName, const std::vector<BYTE>& buffer, ImageDescriptor& descriptor)
{
      auto dumpBuffer = State().codecs.bypassEncoder->Encode(buffer, descriptor);
      StoreMemoryToFile(dumpBuffer, fileName);
}
