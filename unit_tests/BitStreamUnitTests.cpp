#include "CppUnitTest.h"
//#include "../source/png/Huffman.h"
//#include "../png/Huffman.cpp"
#include "../source/bitstream/BitStream.h"
//#include "../png/BitStream.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BitStreamDecodeUnitTests
{
	TEST_CLASS(BitStreamBasicTest)
	{
		TEST_METHOD(BasicUsage)
		{
			BYTE input[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((BYTE)0x00, bitStream.GetByte(0));
			Assert::AreEqual((BYTE)0x0F, bitStream.GetByte(15));
			Assert::AreEqual((size_t)16, bitStream.Size());
		}

		TEST_METHOD(ReadByteByBits1)
		{
			// This should be (0001 0001)
			BYTE input[] = { 0x11, 0x11 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((BYTE) 0x11, bitStream.GetByte());
			Assert::AreEqual((UINT) 0x11, bitStream.GetBits(8));
		}

		TEST_METHOD(ReadByteByBits2)
		{
			// This should be (0001 0001)
			BYTE input[] = { 0x00, 0x13, 0xA0 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x00, bitStream.GetBits(8));
			Assert::AreEqual((UINT)0x13, bitStream.GetBits(8));
			Assert::AreEqual((UINT)0xA0, bitStream.GetBits(8));
		}

		TEST_METHOD(ReadMoreThanByte)
		{
			// This should be (0001 0001)
			BYTE input[] = { 0x00, 0x80 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x00, bitStream.GetBits(9));
			Assert::AreEqual((UINT)0x40, bitStream.GetBits(7));
		}

		TEST_METHOD(RealZLibHeader)
		{
			BYTE input[] = { 0x28, 0x53 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((BYTE)0x28, bitStream.GetByte());
			Assert::AreEqual((BYTE)0x53, bitStream.GetByte());
		}

		TEST_METHOD(RealZlibData)
		{
			BYTE input[] = { 0x78, 0x5e, 0xed, 0x5a, 0x79, 0x70, 0x55, 0x65 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((BYTE)0x78, bitStream.GetByte());
			Assert::AreEqual((BYTE)0x5e, bitStream.GetByte());
			Assert::AreEqual((UINT)1, bitStream.GetBits(1));
			Assert::AreEqual((UINT)2, bitStream.GetBits(2));
			Assert::AreEqual((UINT)286, bitStream.GetBits(5) + 257);
			Assert::AreEqual((UINT)27, bitStream.GetBits(5) + 1);
			Assert::AreEqual((UINT)14, bitStream.GetBits(4) + 4);
		}

		TEST_METHOD(FixLengthCodeLengths)
		{
			BYTE input[] = { 0x78, 0x5e, 0xed, 0x5a, 0x79, 0x70, 0x55, 0x65, 0x9e, 0xd5, 0xbf, 0xba };
			UINT expected[] = { 4, 7, 0, 4, 3, 5, 2, 5, 2, 6, 4, 7, 4, 5 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			// We need to progress to get to the code lengths
			bitStream.GetByte();
			bitStream.GetByte();
			bitStream.GetBits(1);
			bitStream.GetBits(2);
			bitStream.GetBits(5);
			bitStream.GetBits(5);
			bitStream.GetBits(4);

			for (int i = 0; i < 14; i++)
			{
				Assert::AreEqual(expected[i], bitStream.GetBits(3));
			}
		}
	};

	TEST_CLASS(BitsWithinByteBounderies)
	{
		TEST_METHOD(ReadBitsFromOneByteZeros)
		{
			BYTE input[] = { 0x00 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			for (int i = 0; i < 8; i++)
			{
				Assert::AreEqual((UINT)0x0, bitStream.GetBits(1));
			}
		}

		TEST_METHOD(ReadBitsFromOneByteOnes)
		{
			BYTE input[] = { 0xFF };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			for (int i = 0; i < 8; i++)
			{
				Assert::AreEqual((UINT)0x1, bitStream.GetBits(1));
			}
		}

		TEST_METHOD(ReadHalfOfByte1)
		{
			BYTE input[] = { 0x11 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadHalfOfByte2)
		{
			BYTE input[] = { 0x81 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x8, bitStream.GetBits(4), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadHalfOfByte3)
		{
			BYTE input[] = { 0x18 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x8, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadAsymetric1)
		{
			BYTE input[] = { 0x9 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(5), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadAsymetric2)
		{
			BYTE input[] = { 0x21 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(5), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadAsymetricNotFullByte1)
		{
			BYTE input[] = { 0x91 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadAsymetricNotFullByte2)
		{
			BYTE input[] = { 0x11 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadThreeTimes1)
		{
			BYTE input[] = { 0x91 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(1), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadThreeTimes2)
		{
			BYTE input[] = { 0x11 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
			Assert::AreEqual((UINT)0x0, bitStream.GetBits(1), L"Second half is incorrect.");
		}
	};

	TEST_CLASS(BitsWithinTwoBytesBounderies)
	{
		TEST_METHOD(ReadBitsFromOneByteZeros)
		{
			BYTE input[] = { 0x00, 0x00 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			for (int i = 0; i < 8; i++)
			{
				Assert::AreEqual((UINT)0x0, bitStream.GetBits(2));
			}
		}

		TEST_METHOD(ReadBitsFromOneByteOnes)
		{
			BYTE input[] = { 0xFF, 0xFF };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			for (int i = 0; i < 8; i++)
			{
				Assert::AreEqual((UINT)0x3, bitStream.GetBits(2));
			}
		}

		TEST_METHOD(ReadBitsFromOneByteZeroOnes)
		{
			BYTE input[] = { 0x55, 0x55 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			for (int i = 0; i < 8; i++)
			{
				Assert::AreEqual((UINT)0x1, bitStream.GetBits(2));
			}
		}

		TEST_METHOD(ReadFullBytesAsBits1)
		{
			BYTE input[] = { 0x01, 0x01 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(8));
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(8));
		}

		TEST_METHOD(ReadFullBytesAsBits2)
		{
			BYTE input[] = { 0x80, 0x80 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x80, bitStream.GetBits(8));
			Assert::AreEqual((UINT)0x80, bitStream.GetBits(8));
		}

		TEST_METHOD(ReadFullBytesAsBits3)
		{
			BYTE input[] = { 0x77, 0x77 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x77, bitStream.GetBits(8));
			Assert::AreEqual((UINT)0x77, bitStream.GetBits(8));
		}

		TEST_METHOD(ReadHalfBytesAsBits3)
		{
			BYTE input[] = { 0x11, 0x11 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4));
		}

		TEST_METHOD(ReadHalfBytesAsBits4)
		{
			BYTE input[] = { 0x88, 0x88 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x8, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x8, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x8, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x8, bitStream.GetBits(4));
		}

		TEST_METHOD(ReadHalfBytesAsBits5)
		{
			BYTE input[] = { 0x18, 0x18 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x8, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x8, bitStream.GetBits(4));
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4));
		}

		TEST_METHOD(ReadAsymetricNotFullByte1)
		{
			BYTE input[] = { 0x91, 0x91 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(1), L"Second half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(1), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadAsymetricNotFullByte2)
		{
			BYTE input[] = { 0x11, 0x11 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
			Assert::AreEqual((UINT)0x0, bitStream.GetBits(1), L"Second half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(3), L"Second half is incorrect.");
			Assert::AreEqual((UINT)0x0, bitStream.GetBits(1), L"Second half is incorrect.");
		}

		TEST_METHOD(ReadAcrossBytes1)
		{
			BYTE input[] = { 0x11, 0x10 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(8), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
		}

		TEST_METHOD(ReadAcrossBytes2)
		{
			BYTE input[] = { 0x01, 0x18 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x80, bitStream.GetBits(8), L"First half is incorrect.");
			Assert::AreEqual((UINT)0x1, bitStream.GetBits(4), L"First half is incorrect.");
		}

		TEST_METHOD(Problem1)
		{
			BYTE input[] = { 181, 18, 237 };

			BitStream bitStream;
			bitStream.SetData(input, ARRAYSIZE(input));

			// Simulate bit offset 1
			bitStream.GetBits(7);
			Assert::AreEqual((UINT)549, bitStream.GetBits(10));
		}
	};
}
