#include <CppUnitTest.h>
#include "../source/ppm/PpmEncoder.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PpmEncoderUnitTests
{
	TEST_CLASS(BasicBehavioralTests)
	{
	public:

		TEST_METHOD(PpmHeaderTest)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{ 255, 0, 0, 0, 255, 0, 0, 0, 255 };
			AssetSuite::ImageDescriptor descriptor = { 3, 1, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			CompareHeader(result);
		}

		TEST_METHOD(OneDigitWidthAndHeight1)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{ 255, 0, 0, 0, 255, 0, 0, 0, 255 };
			AssetSuite::ImageDescriptor descriptor = { 3, 1, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			CompareDimentions("3", "1", result);
		}

		TEST_METHOD(OneDigitWidthAndHeight2)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{ 255, 0, 0, 0, 255, 0, 0, 0, 255 };
			AssetSuite::ImageDescriptor descriptor = { 1, 3, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			CompareDimentions("1", "3", result);
		}

		TEST_METHOD(OneDigitWidthAndHeight3)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input(512 * 512 * 3, 128);
			AssetSuite::ImageDescriptor descriptor = { 512, 512, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			CompareDimentions("512", "512", result);
		}

		TEST_METHOD(MaxColorValue)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{ 255, 0, 0, 0, 255, 0, 0, 0, 255 };
			AssetSuite::ImageDescriptor descriptor = { 1, 3, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			std::string expected{ "P3\n1 3\n255\n" };
			Compare(expected, result);
		}

		TEST_METHOD(ActualValues1)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{ 255, 0, 0, 0, 255, 0, 0, 0, 255 };
			AssetSuite::ImageDescriptor descriptor = { 1, 3, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			std::string expected{ "P3\n1 3\n255\n255 0 0 \n0 255 0 \n0 0 255 \n" };
			Compare(expected, result);
		}

		TEST_METHOD(ActualValues2)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{ 255, 0, 0, 0, 255, 0, 0, 0, 255 };
			AssetSuite::ImageDescriptor descriptor = { 3, 1, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			std::string expected{ "P3\n3 1\n255\n255 0 0 0 255 0 0 0 255 \n" };
			Compare(expected, result);
		}

		TEST_METHOD(ActualValues3)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{ 255, 0, 0, 0, 255, 0, 0, 0, 255, 1, 2, 3, 1, 2, 3, 1, 2, 3 };
			AssetSuite::ImageDescriptor descriptor = { 3, 2, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			std::string expected{ "P3\n3 2\n255\n255 0 0 0 255 0 0 0 255 \n1 2 3 1 2 3 1 2 3 \n" };
			Compare(expected, result);
		}

		TEST_METHOD(ActualValues4)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{ 255, 0, 0, 0, 255, 0, 0, 0, 255, 1, 2, 3, 1, 2, 3, 1, 2, 3 };
			AssetSuite::ImageDescriptor descriptor = { 2, 3, AssetSuite::ImageFormat::RGB8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			std::string expected{ "P3\n2 3\n255\n255 0 0 0 255 0 \n0 0 255 1 2 3 \n1 2 3 1 2 3 \n" };
			Compare(expected, result);
		}
		TEST_METHOD(ActualValuesRGBA8Test1)
		{
			AssetSuite::PpmEncoder ppmEncoder;
			std::vector<BYTE> input{
				255, 0, 0, 255,
				0, 255, 0, 255,
				0, 0, 255, 255,
				1, 2, 3, 255,
				1, 2, 3, 255,
				1, 2, 3, 255
			};
			AssetSuite::ImageDescriptor descriptor = { 2, 3, AssetSuite::ImageFormat::RGBA8 };
			auto result = ppmEncoder.Encode(input, descriptor);

			std::string expected{ "P3\n2 3\n255\n255 0 0 0 255 0 \n0 0 255 1 2 3 \n1 2 3 1 2 3 \n" };
			Compare(expected, result);
		}
	private:
		// This could be a template!
		void Compare(const std::vector<char>& expected, const std::vector<BYTE>& actual)
		{
			unsigned int index = 0;
			if (actual.size() < expected.size())
			{
				Assert::Fail(L"Not enough actual data to perform comparison");
			}
			
			for (auto it = expected.begin(); it != expected.end(); it++)
			{
				Assert::AreEqual(*it, (char)actual[index]);
				index++;
			}
		}
		void Compare(const std::string& expected, const std::vector<BYTE>& actual)
		{
			unsigned int index = 0;
			if (actual.size() < expected.size())
			{
				Assert::Fail(L"Not enough actual data to perform comparison");
			}

			for (auto it = expected.begin(); it != expected.end(); it++)
			{
				Assert::AreEqual(*it, (char)actual[index]);
				index++;
			}
		}
		void CompareHeader(const std::vector<BYTE>& actual)
		{
			std::string expected{ "P3\n" };
			if (actual.size() < expected.size())
			{
				Assert::Fail(L"Not enough actual data to perform PPM header comparison");
			}

			for (int i = 0; i < expected.size(); i++)
			{
				Assert::AreEqual(expected[i], (char)actual[i], L"The PPM header is incorrect");
			}
		}
		void CompareDimentions(const std::string& expectedWidth, const std::string& expectedHeight, const std::vector<BYTE>& actual)
		{
			std::string expected = expectedWidth + " " + expectedHeight + "\n";
			if (actual.size() < expected.size() + 3)
			{
				Assert::Fail(L"Not enough actual data to perform PPM dimentions comparison");
			}
			for (int i = 0; i < expected.size(); i++)
			{
				Assert::AreEqual(expected[i], (char)actual[i + 3], L"The PPM dimentions are incorrect");
			}
		}
	};
}