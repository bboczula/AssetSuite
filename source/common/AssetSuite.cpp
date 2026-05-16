#pragma warning (disable : 4251)

#include "AssetSuite.h"
#include "AssetSuiteContext.h"

#include "../runtime/AssetSuiteRuntime.h"
#include "../runtime/AssetSuiteRuntimeState.h"
#include "../wavefront/ModelLoader.h"
#include "../bmp/BmpDecoder.h"
#include "../png/PngDecoder.h"
#include "../ppm/PpmEncoder.h"
#include "../bypass/BypassEncoder.h"

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

	try
	{
		*outContext = new AssetSuiteContext_t(normalizedDesc);
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

AssetSuite::Result AssetSuite::DestroyContext(ContextHandle* context)
{
	if (!context || !*context)
	{
		return Result::ErrorInvalidContext;
	}

	delete *context;
	*context = nullptr;
	return Result::Success;
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
            decoder = state.codecRegistry.ResolveImageDecoder(state.fileInfo.extension);
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
            decoder = state.codecRegistry.ResolveMeshDecoder(state.fileInfo.extension);
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
      auto groupOffset = state.modelLoader->GetGroupOffset(meshName);
      auto groupSize = state.modelLoader->GetGroupSize(meshName);
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
                  auto face = state.modelLoader->GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto vertex = state.modelLoader->GetVertex(face.vertexIndex[j]);
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
                  auto face = state.modelLoader->GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto normal = state.modelLoader->GetNormal(face.normalIndex[j]);
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
                  auto face = state.modelLoader->GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto tangent = state.modelLoader->GetTangent(face.normalIndex[j]);
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
                  auto face = state.modelLoader->GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto texCoord = state.modelLoader->GetTextureCoord(face.textureIndex[j]);
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
      state.rawBuffer = state.ppmEncoder->Encode(buffer, imageDescriptor);
      StoreMemoryToFile(state.rawBuffer, filePathAndName);
}

AssetSuite::ErrorCode AssetSuite::Manager::LoadFileToMemory(const std::string& fileName, bool isBinary)
{
      auto& state = State();
      const ErrorCode result = state.fileLoader.LoadToMemory(fileName, isBinary, state.rawBuffer);
      if (result != ErrorCode::OK)
      {
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
      auto dumpBuffer = State().bypassEncoder->Encode(buffer, descriptor);
      StoreMemoryToFile(dumpBuffer, fileName);
}
