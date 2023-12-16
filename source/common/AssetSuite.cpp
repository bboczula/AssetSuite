#include "AssetSuite.h"

#include <filesystem>

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

BYTE* AssetSuite::Manager::LoadMeshFromFile(const std::string& filePathAndName, MeshDescriptor& meshDescriptor)
{
	modelLoader->LoadFromFile(filePathAndName);
      return nullptr;
}

AssetSuite::ErrorCode AssetSuite::Manager::LoadImageFromFile(const char* filePathAndName, ImageDescriptor& imageDescriptor, std::vector<BYTE>& output)
{
      std::filesystem::path fullName(filePathAndName);
      std::filesystem::path extension = fullName.extension();
      LoadFileToMemory(filePathAndName);
      //std::vector<BYTE> output;
      ErrorCode result;
      if (extension.compare(".bmp") == 0)
      {
            bmpDecoder->Decode(output, buffer.data(), imageDescriptor);
            result = ErrorCode::OK;
      }
      else if (extension.compare(".png") == 0)
      {
#if 0
            auto dumpBuffer = bypassEncoder->Encode(buffer, imageDescriptor);
            StoreMemoryToFile(dumpBuffer, "dump.txt");
            std::vector<BYTE> actualBuffer;
            pngDecoder->Decode(actualBuffer, buffer.data(), imageDescriptor);
            auto dumpEncodedBuffer = bypassEncoder->Encode(actualBuffer, imageDescriptor);
            StoreMemoryToFile(dumpEncodedBuffer, "expected.txt");
#endif
            
            auto error = pngDecoder->Decode(output, buffer.data(), imageDescriptor);
            result = error == DecoderError::NoDecoderError ? ErrorCode::OK : ErrorCode::ColorTypeNotSupported;
      }
      return result;
}

void AssetSuite::Manager::StoreMeshToFile(const std::string& filePathAndName, BYTE* buffer, const MeshDescriptor& imageDescriptor)
{
}

void AssetSuite::Manager::StoreImageToFile(const std::string& filePathAndName, const std::vector<BYTE>& buffer, const ImageDescriptor& imageDescriptor)
{
      this->buffer = ppmEncoder->Encode(buffer, imageDescriptor);
      StoreMemoryToFile(this->buffer, filePathAndName);
}

void AssetSuite::Manager::LoadFileToMemory(const std::string& fileName)
{
      // Clear the buffer, since it might have something in it
      buffer.clear();

      //load and decode
      std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

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
            buffer.resize((size_t)size);
            file.read((char*)(&buffer[0]), size);
      }
      else
      {
            buffer.clear();
      }
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
