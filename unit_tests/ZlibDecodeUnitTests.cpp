#include "CppUnitTest.h"
#include "../source/zlib/Huffman.h"
//#include "../png/Huffman.cpp"
#include "../source/bitstream/BitStream.h"
//#include "../png/BitStream.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ZlibDecodeUnitTests
{
	TEST_CLASS(BasicBehavioralTests)
	{
	public:
		
		TEST_METHOD(BuildCodesUsingRfcAlgorithm)
		{
			SymbolList symbols = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
			std::vector<UINT> lengths = { 3, 3, 3, 3, 3, 2, 4, 4 };
			std::vector<UINT> expectedCodes = { 2, 3, 4, 5, 6, 0, 14, 15 };

			HuffmanTree tree;
			tree.BuildFromLengths(symbols, lengths);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				symbol_t outputSymbol;
				AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], lengths[i]);
				Assert::AreEqual(AssetSuite::Result::OK == result, true);
				Assert::AreEqual(symbols[i], outputSymbol);
			}
		}

		TEST_METHOD(BuildCodesUsingInternetAlgorithm)
		{
			SymbolList symbols = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
			std::vector<UINT> lengths = { 3, 3, 3, 3, 3, 2, 4, 4 };
			std::vector<UINT> expectedCodes = { 2, 3, 4, 5, 6, 0, 14, 15 };

			HuffmanTree tree;
			tree.BuildFromLengths(symbols, lengths);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				symbol_t outputSymbol;
				AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], lengths[i]);
				Assert::AreEqual(AssetSuite::Result::OK == result, true);
				Assert::AreEqual(symbols[i], outputSymbol);
			}
		}

		TEST_METHOD(BuildCodesFromRfc)
		{
			SymbolList symbols = { 'A', 'B', 'C', 'D' };
			std::vector<UINT> lengths = { 2, 1, 3, 3 };
			std::vector<UINT> expectedCodes = { 2, 0, 6, 7 };

			HuffmanTree tree;
			tree.BuildFromLengths(symbols, lengths);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				symbol_t outputSymbol;
				AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], lengths[i]);
				Assert::AreEqual(AssetSuite::Result::OK == result, true);
				Assert::AreEqual(symbols[i], outputSymbol);
			}
		}

		TEST_METHOD(BuildFromCodeLengths)
		{
			SymbolList symbols = { 0x00, 0x01, 0x02, 0x03 };
			std::vector<UINT> lengths = { 2, 1, 3, 3 };
			std::vector<UINT> expectedCodes = { 2, 0, 6, 7 };

			HuffmanTree tree;
			tree.BuildFromLengths(lengths);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				symbol_t outputSymbol;
				AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], lengths[i]);
				Assert::AreEqual(AssetSuite::Result::OK == result, true, L"GetSymbol() failed for some reason");
				Assert::AreEqual(symbols[i], outputSymbol);
			}
		}

		TEST_METHOD(BuildFromCodeLengthsAndSymbols)
		{
			SymbolList symbols = { 'A', 'B', 'C', 'D' };
			std::vector<UINT> lengths = { 2, 1, 3, 3 };
			std::vector<UINT> expectedCodes = { 2, 0, 6, 7 };

			HuffmanTree tree;
			tree.BuildFromLengths(symbols, lengths);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				symbol_t outputSymbol;
				AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], lengths[i]);
				Assert::AreEqual(AssetSuite::Result::OK == result, true, L"GetSymbol() failed for some reason");
				Assert::AreEqual(symbols[i], outputSymbol);
			}
		}

		TEST_METHOD(BuildJustFromCodesFromRealData)
		{
			SymbolList symbols =       { 0,  1,  2,  3,  4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
			std::vector<UINT> lengths =       { 2,  4,  4,  4,  4, 2, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  3,  3 };
			std::vector<UINT> expectedLengths = { 2,  4,  4,  4,  4, 2, 3, 3 };
			std::vector<UINT> expectedCodes = { 0, 12, 13, 14, 15, 1, 4,  5 };
			SymbolList expectedSymbols = { 0, 1, 2, 3, 4, 5, 17, 18 };

			HuffmanTree tree;
			tree.BuildFromLengths(symbols, lengths);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				symbol_t outputSymbol;
				AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], expectedLengths[i]);
				Assert::AreEqual(AssetSuite::Result::OK == result, true, L"GetSymbol() failed for some reason");
				Assert::AreEqual(expectedSymbols[i], outputSymbol);
			}
		}

		TEST_METHOD(BuildFromFrequenciesFromWikipedia)
		{
			// Source: https://en.wikipedia.org/wiki/Huffman_coding
			// Note: They don't use the canonical form, I had to change the result

			SymbolList symbols = { ' ', 'a', 'e', 'f', 'h', 'i', 'm', 'n', 's', 't', 'l', 'o', 'p', 'r', 'u', 'x' };
			std::vector<UINT> frequencies = { 7, 4, 4, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1 };
			std::vector<UINT> expectedCodes = { 0, 1, 2, 6, 7, 8, 9, 10, 11, 12, 26, 27, 28, 29, 30, 31 };
			std::vector<UINT> lengths = { 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5 };

			HuffmanTree tree;
			tree.BuildFromFrequencies(symbols, frequencies);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				if (lengths[i] != 0)
				{
					symbol_t outputSymbol;
					AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], lengths[i]);
					Assert::AreEqual(AssetSuite::Result::OK == result, true, L"Get symbol returned the error, even though it shouldn't");
					Assert::AreEqual(symbols[i], outputSymbol, L"Incorrect symbol was returned");
				}
			}
		}

		TEST_METHOD(BuildJustFromFrequencies)
		{
			SymbolList symbols = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
			std::vector<UINT> frequencies = { 7, 4, 4, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1 };
			std::vector<UINT> expectedCodes = { 0, 1, 2, 6, 7, 8, 9, 10, 11, 12, 26, 27, 28, 29, 30, 31 };
			std::vector<UINT> lengths = { 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5 };

			HuffmanTree tree;
			tree.BuildFromFrequencies(frequencies);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				if (lengths[i] != 0)
				{
					symbol_t outputSymbol;
					AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], lengths[i]);
					Assert::AreEqual(AssetSuite::Result::OK == result, true, L"Get symbol returned the error, even though it shouldn't");
					Assert::AreEqual(symbols[i], outputSymbol, L"Incorrect symbol was returned");
				}
			}
		}

		TEST_METHOD(KillBill)
		{
			SymbolList symbols = { 'l', 'i','b', 'k' };
			std::vector<UINT> frequencies = { 4, 2, 1, 1 };
			std::vector<UINT> expectedCodes = { 0, 2, 6, 7 };
			std::vector<UINT> expectedLengths = { 1, 2, 3, 3 };

			HuffmanTree tree;
			tree.BuildFromFrequencies(symbols, frequencies);

			for (UINT i = 0; i < expectedCodes.size(); i++)
			{
				if (expectedLengths[i] != 0)
				{
					symbol_t outputSymbol;
					AssetSuite::Result result = tree.GetSymbol(outputSymbol, expectedCodes[i], expectedLengths[i]);
					Assert::AreEqual(AssetSuite::Result::OK == result, true, L"Get symbol returned the error, even though it shouldn't");
					Assert::AreEqual(symbols[i], outputSymbol, L"Incorrect symbol was returned");
				}
			}
		}
	};

	TEST_CLASS(ErrorHandlingTests)
	{
		TEST_METHOD(SymbolNotFound)
		{
			SymbolList symbols = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
			std::vector<UINT> lengths = { 3, 3, 3, 3, 3, 2, 4, 4 };
			std::vector<UINT> expectedCodes = { 2, 3, 4, 5, 6, 0, 14, 15 };

			HuffmanTree tree;
			tree.BuildFromLengths(symbols, lengths);
			symbol_t outputSymbol;
			AssetSuite::Result result = tree.GetSymbol(outputSymbol, 0, 3);
			Assert::AreEqual(AssetSuite::Result::SymbolNotFound == result, true, L"Symbol was found, while it shouldn't");
		}

		TEST_METHOD(CodeTableIsEmpty)
		{
			HuffmanTree tree;
			symbol_t outputSymbol;
			AssetSuite::Result result = tree.GetSymbol(outputSymbol, 0, 3);
			Assert::AreEqual(AssetSuite::Result::CodeTableIsEmpty == result, true, L"Code Table is not empty, while it should");
		}

		TEST_METHOD(SymbolNotFoundForZeroLength)
		{
			SymbolList symbols = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
			std::vector<UINT> lengths = { 3, 3, 3, 3, 3, 2, 4, 4 };
			std::vector<UINT> expectedCodes = { 2, 3, 4, 5, 6, 0, 14, 15 };

			HuffmanTree tree;
			tree.BuildFromLengths(symbols, lengths);
			symbol_t outputSymbol;
			AssetSuite::Result result = tree.GetSymbol(outputSymbol, 2, 0);
			Assert::AreEqual(AssetSuite::Result::SymbolNotUsed == result, true, L"Symbol was found, even though we asked for code length zero");
		}

		TEST_METHOD(SymbolNotUsed)
		{
			SymbolList symbols = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
			std::vector<UINT> lengths = { 2, 4, 4, 4, 4, 2, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  3 };
			std::vector<UINT> expectedCodes = { 0, 12, 13, 14, 15, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 5 };

			HuffmanTree tree;
			tree.BuildFromLengths(symbols, lengths);
			symbol_t outputSymbol;
			AssetSuite::Result result = tree.GetSymbol(outputSymbol, 0, 0);
			Assert::AreEqual(AssetSuite::Result::SymbolNotUsed == result, true);
		}
	};
}
