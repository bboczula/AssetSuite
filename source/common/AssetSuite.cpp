#pragma warning (disable : 4251)

#include "AssetSuite.h"

#include "../wavefront/ModelLoader.h"
#include "../wavefront/Image.h"
#include "../bmp/BmpDecoder.h"
#include "../png/PngDecoder.h"
#include "../ppm/PpmEncoder.h"
#include "../bypass/BypassEncoder.h"

AssetSuite::Manager::Manager() : modelLoader(nullptr)
{
	modelLoader = new ModelLoader;
      bmpDecoder = new BmpDecoder;
      pngDecoder = new PngDecoder;
      ppmEncoder = new PpmEncoder;
      bypassEncoder = new BypassEncoder;

      imageDecoders[(size_t)ImageDecoders::BMP] = bmpDecoder;
      imageDecoders[(size_t)ImageDecoders::PNG] = pngDecoder;

      meshDecoders[(size_t)MeshDecoders::WAVEFRONT] = modelLoader;
}

AssetSuite::Manager::~Manager()
{
      if (bypassEncoder)
      {
            delete bypassEncoder;
      }

      if (ppmEncoder)
      {
            delete ppmEncoder;
      }

      if (pngDecoder)
      {
            delete pngDecoder;
      }

      if (bmpDecoder)
      {
            delete bmpDecoder;
      }

	if (modelLoader)
	{
		delete modelLoader;
	}
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
      fileInfo.fullName = filePathAndName;
      fileInfo.extension = fileInfo.fullName.extension();
      return LoadFileToMemory(filePathAndName);
}

AssetSuite::ErrorCode AssetSuite::Manager::ImageDecode(ImageDecoders decoder)
{
      if (rawBuffer.empty())
      {
            return ErrorCode::RawBufferIsEmpty;
      }

      if (decoder == ImageDecoders::Auto)
      {
            if (fileInfo.extension.compare(".bmp") == 0)
            {
                  decoder = ImageDecoders::BMP;
            }
            else if (fileInfo.extension.compare(".png") == 0)
            {
                  decoder = ImageDecoders::PNG;
            }
            else
            {
                  return ErrorCode::FileTypeNotSupported;
            }
      }

      ImageDescriptor descriptor;
      auto error = imageDecoders[(size_t)decoder]->Decode(decodedBuffer, rawBuffer.data(), descriptor);

      imageInfo.width = descriptor.width;
      imageInfo.height = descriptor.height;
      imageInfo.format = descriptor.format;

      return error ? ErrorCode::OK : ErrorCode::Undefined;
}

AssetSuite::ErrorCode AssetSuite::Manager::ImageGet(OutputFormat format, std::vector<BYTE>& output, ImageDescriptor& descriptor)
{
      if (decodedBuffer.empty())
      {
            return ErrorCode::DecodedBufferIsEmpty;
      }

      output = decodedBuffer;
      descriptor.width = imageInfo.width;
      descriptor.height = imageInfo.height;
      descriptor.format = imageInfo.format;

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
      fileInfo.fullName = filePathAndName;
      fileInfo.extension = fileInfo.fullName.extension();
      return LoadFileToMemory(filePathAndName, false);
}

AssetSuite::ErrorCode AssetSuite::Manager::MeshDecode(MeshDecoders decoder)
{
      if (rawBuffer.empty())
      {
            return ErrorCode::RawBufferIsEmpty;
      }

      if (decoder == MeshDecoders::Auto)
      {
            if (fileInfo.extension.compare(".obj") == 0)
            {
                  decoder = MeshDecoders::WAVEFRONT;
            }
            else
            {
                  return ErrorCode::FileTypeNotSupported;
            }
      }

      std::vector<BYTE> output;
      MeshDescriptor descriptor;
      auto error = meshDecoders[(size_t)decoder]->Decode(output, rawBuffer.data(), descriptor);
      return error ? ErrorCode::OK : ErrorCode::Undefined;
}

AssetSuite::ErrorCode AssetSuite::Manager::MeshGet(const char* meshName, MeshOutputFormat format, std::vector<FLOAT>& output, MeshDescriptor& descriptor)
{
      // Fetch the group data
      auto groupOffset = modelLoader->GetGroupOffset(meshName);
      auto groupSize = modelLoader->GetGroupSize(meshName);
      meshInfo.numOfVertices = groupSize;
      meshInfo.numOfIndices = groupSize;
      descriptor.numOfVertices = meshInfo.numOfVertices;
      descriptor.numOfIndices = meshInfo.numOfIndices;

      if (format == MeshOutputFormat::POSITION)
      {
            // Each face has three vertices, each vertex has four elements
            output.resize(groupSize * 3 * 4);
            for (UINT i = 0; i < groupSize; i++)
            {
                  auto face = modelLoader->GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto vertex = modelLoader->GetVertex(face.vertexIndex[j]);
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
                  auto face = modelLoader->GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto normal = modelLoader->GetNormal(face.normalIndex[j]);
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
                  auto face = modelLoader->GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto tangent = modelLoader->GetTangent(face.normalIndex[j]);
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
                  auto face = modelLoader->GetFace(i + groupOffset);
                  for (int j = 0; j < 3; j++)
                  {
                        auto texCoord = modelLoader->GetTextureCoord(face.textureIndex[j]);
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
      DumpBuffer("rawBuffer.txt", rawBuffer, descriptor);
      return ErrorCode::OK;
}

AssetSuite::ErrorCode AssetSuite::Manager::DumpDecodedBuffer()
{
      ImageDescriptor descriptor;
      DumpBuffer("decodedBuffer.txt", decodedBuffer, descriptor);
      return ErrorCode::OK;
}

void AssetSuite::Manager::StoreImageToFile(const std::string& filePathAndName, const std::vector<BYTE>& buffer, const ImageDescriptor& imageDescriptor)
{
      this->rawBuffer = ppmEncoder->Encode(buffer, imageDescriptor);
      StoreMemoryToFile(this->rawBuffer, filePathAndName);
}

AssetSuite::ErrorCode AssetSuite::Manager::LoadFileToMemory(const std::string& fileName, bool isBinary)
{
      // Check if file exists
      if (!std::filesystem::exists(fileInfo.fullName))
      {
            return ErrorCode::NonExistingFile;
      }

      // Clear the buffer, since it might have something in it
      rawBuffer.clear();

      //load and decode
      std::ifstream file(fileName.c_str(), std::ios::in | std::ios::ate | std::ios::binary);

      // Figure out the file size
      // - this type is an implementation-defined signed integral type used to represent the number of
      //   characters transered in an I/O opration or the size of I/O buffer. It is usead as a ssinged counterpart of the std:size_T
      std::streamsize size = 0;

      // - seekg sets the position of the next character to be extracted from the input stream
      // - it returns the istream object
      // - first parameter is the offset value, relative to the second parameter
      // - second parameter can take three values: beginning, current or end of the current stream
      // - this seems to set the next character to read as the last character in the file
      if (file.seekg(0, std::ios::end).good())
      {
            // tellg() returns the position of the currenct character in the input stream
            // the return type is the streampos
            // effectively this returns the position of the last character
            size = file.tellg();
      }

      // this seems to set the next character to read as the first character
      if (file.seekg(0, std::ios::beg).good())
      {
            // tellg() function returns the position of the first character
            // you calculate the size of the file by substracting position of the last character from the first character
            size -= file.tellg();
      }

      //read contents of the file into the vector
      if (size > 0)
      {
            rawBuffer.resize((size_t)size);
            file.read((char*)(&rawBuffer[0]), size);
            if (!isBinary)
            {
                  // Needed for stringstream to work properly.
                  rawBuffer.push_back('\0');
            }
      }
      else
      {
            rawBuffer.clear();
      }

      return ErrorCode::OK;
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
      auto dumpBuffer = bypassEncoder->Encode(buffer, descriptor);
      StoreMemoryToFile(dumpBuffer, fileName);
}