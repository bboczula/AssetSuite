#include <CppUnitTest.h>
#include "../source/bmp/BmpDecoder.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BmpDecoderUnitTests
{
	TEST_CLASS(BasicBehavioralTests)
	{
	public:

		TEST_METHOD(BmpOutputBufferSizeTest)
		{
			// Test Variables
			const UINT WIDTH = 10;
			const UINT HEIGHT = 10;
			const UINT BPP = 24;

			// Why this has to have this mirrored order of bytes?
			std::vector<BYTE> input(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

			BITMAPFILEHEADER bmpFileHeader;
			bmpFileHeader.bfType = 0x4d42;	// The file type; must be BM.
			bmpFileHeader.bfSize = 0x00000176;	// The size, in bytes, of the bitmap file.
			bmpFileHeader.bfReserved1 = 0;	// Reserved; must be zero.
			bmpFileHeader.bfReserved2 = 0;	// Reserved; must be zero.
			bmpFileHeader.bfOffBits = 54;		// The offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
			memcpy(input.data(), &bmpFileHeader, sizeof(BITMAPFILEHEADER));

			BITMAPINFOHEADER bmpInfoHeader;
			bmpInfoHeader.biSize = 0x00000028;
			bmpInfoHeader.biWidth = WIDTH;
			bmpInfoHeader.biHeight = HEIGHT;
			bmpInfoHeader.biPlanes = 0x0001;
			bmpInfoHeader.biBitCount = BPP;
			bmpInfoHeader.biCompression = 0x0;
			bmpInfoHeader.biSizeImage = 0x00000140;
			bmpInfoHeader.biXPelsPerMeter = 0x0;
			bmpInfoHeader.biYPelsPerMeter = 0x0;
			bmpInfoHeader.biClrUsed = 0x0;
			bmpInfoHeader.biClrImportant = 0x0;
			memcpy(input.data() + sizeof(BITMAPFILEHEADER), &bmpInfoHeader, sizeof(BITMAPINFOHEADER));

			for (int i = 0; i < HEIGHT; i++)
			{
				for (int j = 0; j < WIDTH; j++)
				{
					input.push_back(64);
					input.push_back(128);
					input.push_back(255);
				}
			}
			AssetSuite::ImageDescriptor imageDescriptor;
			AssetSuite::BmpDecoder bmpDecoder;
			std::vector<BYTE> result;
			auto error = bmpDecoder.Decode(result, input.data(), imageDescriptor);
			Assert::AreEqual(true, AssetSuite::DecoderError::NoDecoderError == error);

			Assert::AreEqual((UINT)WIDTH, imageDescriptor.width);
			Assert::AreEqual((UINT)HEIGHT, imageDescriptor.height);
			Assert::AreEqual((size_t)300, result.size());
		}

		TEST_METHOD(BmpOutputBufferValuesTest)
		{
			// Test Variables
			const UINT WIDTH = 10;
			const UINT HEIGHT = 10;
			const UINT BPP = 24;

			// Why this has to have this mirrored order of bytes?
			std::vector<BYTE> input(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

			BITMAPFILEHEADER bmpFileHeader;
			bmpFileHeader.bfType = 0x4d42;	// The file type; must be BM.
			bmpFileHeader.bfSize = 0x00000176;	// The size, in bytes, of the bitmap file.
			bmpFileHeader.bfReserved1 = 0;	// Reserved; must be zero.
			bmpFileHeader.bfReserved2 = 0;	// Reserved; must be zero.
			bmpFileHeader.bfOffBits = 54;		// The offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
			memcpy(input.data(), &bmpFileHeader, sizeof(BITMAPFILEHEADER));

			BITMAPINFOHEADER bmpInfoHeader;
			bmpInfoHeader.biSize = 40;		// Specifies the number of bytes required by the structure.
			bmpInfoHeader.biWidth = WIDTH;	// Specifies the width of the bitmap, in pixels.
			bmpInfoHeader.biHeight = HEIGHT;	// Specifies the height of the bitmap, in pixels.
			bmpInfoHeader.biPlanes = 1;		// Specifies the number of planes for the target device. This value must be set to 1.
			bmpInfoHeader.biBitCount = BPP;	// Specifies the number of bits per pixel (bpp).
			bmpInfoHeader.biCompression = 0;	// Something for compression
			bmpInfoHeader.biSizeImage = 320;	// Specifies the size, in bytes, of the image. This can be set to 0 for uncompressed RGB bitmaps.
			bmpInfoHeader.biXPelsPerMeter = 0;	// Specifies the horizontal resolution, in pixels per meter
			bmpInfoHeader.biYPelsPerMeter = 0;	// Specifies the vertical resolution, in pixels per meter
			bmpInfoHeader.biClrUsed = 0;		// Specifies the number of color indices in the color table that are actually used by the bitmap
			bmpInfoHeader.biClrImportant = 0;	// Specifies the number of color indices that are considered important for displaying the bitmap
			memcpy(input.data() + sizeof(BITMAPFILEHEADER), &bmpInfoHeader, sizeof(BITMAPINFOHEADER));

			std::vector<BYTE> expected;
			for (int i = 0; i < HEIGHT; i++)
			{
				for (int j = 0; j < WIDTH; j++)
				{
					// BMP format is BGR
					input.push_back(64);
					input.push_back(128);
					input.push_back(255);

					// We are expecting the RGB format
					expected.push_back(255);
					expected.push_back(128);
					expected.push_back(64);
				}
				// Sometimes we need alignemnt
				input.push_back(0);
				input.push_back(0);
			}
			AssetSuite::ImageDescriptor imageDescriptor;
			AssetSuite::BmpDecoder bmpDecoder;
			std::vector<BYTE> actual;
			auto error = bmpDecoder.Decode(actual, input.data(), imageDescriptor);
			Assert::AreEqual(true, AssetSuite::DecoderError::NoDecoderError == error);

			Assert::AreEqual((UINT)WIDTH, imageDescriptor.width);
			Assert::AreEqual((UINT)HEIGHT, imageDescriptor.height);
			for (int i = 0; i < 300; i++)
			{
				Assert::AreEqual(expected[i], actual[i]);
			}
		}

		TEST_METHOD(BmpOutputFormatTest)
		{
			// Test Variables
			const UINT WIDTH = 10;
			const UINT HEIGHT = 10;
			const UINT BPP = 24;

			// Why this has to have this mirrored order of bytes?
			std::vector<BYTE> input(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

			BITMAPFILEHEADER bmpFileHeader;
			bmpFileHeader.bfType = 0x4d42;	// The file type; must be BM.
			bmpFileHeader.bfSize = 0x00000176;	// The size, in bytes, of the bitmap file.
			bmpFileHeader.bfReserved1 = 0;	// Reserved; must be zero.
			bmpFileHeader.bfReserved2 = 0;	// Reserved; must be zero.
			bmpFileHeader.bfOffBits = 54;		// The offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
			memcpy(input.data(), &bmpFileHeader, sizeof(BITMAPFILEHEADER));

			BITMAPINFOHEADER bmpInfoHeader;
			bmpInfoHeader.biSize = 0x00000028;
			bmpInfoHeader.biWidth = WIDTH;
			bmpInfoHeader.biHeight = HEIGHT;
			bmpInfoHeader.biPlanes = 0x0001;
			bmpInfoHeader.biBitCount = BPP;
			bmpInfoHeader.biCompression = 0x0;
			bmpInfoHeader.biSizeImage = 0x00000140;
			bmpInfoHeader.biXPelsPerMeter = 0x0;
			bmpInfoHeader.biYPelsPerMeter = 0x0;
			bmpInfoHeader.biClrUsed = 0x0;
			bmpInfoHeader.biClrImportant = 0x0;
			memcpy(input.data() + sizeof(BITMAPFILEHEADER), &bmpInfoHeader, sizeof(BITMAPINFOHEADER));

			for (int i = 0; i < HEIGHT; i++)
			{
				for (int j = 0; j < WIDTH; j++)
				{
					input.push_back(64);
					input.push_back(128);
					input.push_back(255);
				}
			}
			AssetSuite::ImageDescriptor imageDescriptor;
			AssetSuite::BmpDecoder bmpDecoder;
			std::vector<BYTE> result;
			auto error = bmpDecoder.Decode(result, input.data(), imageDescriptor);
			Assert::AreEqual(true, AssetSuite::DecoderError::NoDecoderError == error);

			Assert::AreEqual(true, AssetSuite::ImageFormat::RGB8 == imageDescriptor.format);
		}
	};
}