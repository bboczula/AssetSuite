#include "ZLib.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstddef>
#include <vector>
#include <list>
#include <algorithm>
#include <Windows.h>
#include <bitset>
#include <cmath>
#include <queue>
#include <map>
#include <span>

#define ENABLE_PRINT 0

// Global Variables
const UINT CODE_LENGTHS_MAX_SIZE = CodeLengthSymbols::COUNT;
static const UINT CodeLengthsSwizzleTable[CODE_LENGTHS_MAX_SIZE] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
static const UINT LiteralLengthAdditionalBits[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
static const UINT LiteralLengthValueOffset[] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
static const UINT DistanceAdditionalBits[] = { 0,0,0,0,1,1,2, 2, 3, 3, 4, 4, 5, 5,  6,  6,  7,  7,  8,  8,   9,   9,  10,  10,  11,  11,  12,   12,   13,   13 };
static const UINT DistanceValueOffset[] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577 };

// This needs to return the result
std::vector<BYTE> ZLib::Decode(const std::string& byteString)
{
	// It has to be even number, assert this
	// You need to return the error

	std::vector<BYTE> outputByteArray;
	outputByteArray.reserve(byteString.length() / 2);

	// You could also skip spaces
	for (auto it = byteString.begin(); it != byteString.end(); it++)
	{
		// Iterate over every element
		if (*it == ' ')
		{
			continue;
		}
		auto b1 = Char2Byte(std::toupper(*it));
		auto b2 = Char2Byte(std::toupper(*(++it)));
		outputByteArray.push_back((b1 << 4) | b2);
	}

	// Decode takes a pointer to BYTE and the size of the buffer
	return Decode(outputByteArray);
}

std::vector<BYTE> ZLib::Decode(const std::vector<BYTE>& byteStream)
{
	return Decode(byteStream.data(), byteStream.size());
}

std::vector<BYTE> ZLib::Decode(const BYTE* data, size_t size)
{
	BitStream bitStream;
	bitStream.SetData(data, size);

	BYTE CMF = bitStream.GetByte();//outputByteArray[0];
	UINT CM = CMF & 0xF;
	UINT CINFO = CMF >> 4;
#if ENABLE_PRINT
	std::cout << "CM (Compression Method): " << CM << std::endl;
	std::cout << "CINFO (Compression Info): " << CINFO << std::endl;

	std::cout << "2: Read the FLG (Flags) byte" << std::endl;
#endif
	BYTE FLG = bitStream.GetByte(); //outputByteArray[1];
	UINT FCHECK = FLG & 0x1F;
	UINT FDICT = (FLG >> 5) & 0x1;
	UINT FLEVEL = FLG >> 6;

#if ENABLE_PRINT
	std::cout << "FCHECK (Check Bits for CMF and FLG): " << FCHECK << std::endl;
	std::cout << "FDICT (Preset Dictionary): " << FDICT << std::endl;
	std::cout << "FLEVEL (Compression Level): " << FLEVEL << std::endl;
#endif

	// This is de-facto deflate
	std::vector<symbol_t> symbolsList;
	Inflate(bitStream, symbolsList);

	// Here you can technically do the filtering
	std::vector<BYTE> output;
	output.reserve(symbolsList.size());
	// We start from 1, since the first byte represents filtering
	for (int i = 0; i < symbolsList.size(); i++)
	{
		output.push_back(symbolsList[i]);
	}

	// Temporarily print the data
	//for (int i = 0; i < 4; i++)
	//{
	//	for (int j = 0; j < 4 * 4 + 1; j++)
	//	{
	//		std::cout << (UINT)output[(i * (4 * 4 + 1)) + j] << " ";
	//	}
	//	std::cout << std::endl;
	//}

	return output;
}

void ZLib::Inflate(BitStream& bitStream, std::vector<symbol_t>& symbolsList)
{
#if ENABLE_PRINT
	std::cout << "2: Read the first DEFLATE block header" << std::endl;
#endif
	UINT BFINAL = 0;
	while (!BFINAL)
	{
		BFINAL = bitStream.GetBits(1);
		UINT BTYPE = bitStream.GetBits(2);
#if ENABLE_PRINT
		std::cout << "BFINAL: " << BFINAL << std::endl;
		std::cout << "BTYPE: " << BTYPE << std::endl;

		std::cout << "  3: Check the compression type" << std::endl;
#endif
		if (BTYPE == 0)
		{
#if ENABLE_PRINT
			std::cout << "  Non-compressed blocks\n";
#endif
		}
		else if (BTYPE <= 2)
		{
#if ENABLE_PRINT
			std::cout << "  Compressed blocks (length and distance codes)\n";
#endif
			HandleCompressedBlock(BTYPE, bitStream, symbolsList);
		}
		else
		{
			std::cerr << "  ERROR: Unsupported BTYPE!\n";
		}
	}
}

void ZLib::HandleCompressedBlock(const UINT& BTYPE, BitStream& bitStream, std::vector<symbol_t>& symbolsList)
{
	std::vector<UINT> literalLengthCodeLengths;
	std::vector<UINT> distanceCodeLengths;
	if (BTYPE == 2)
	{
		// DYNAMIC HUFFMAN CODES
		// Read some header bits
		UINT HLIT = bitStream.GetBits(5) + 257;
		UINT HDIST = bitStream.GetBits(5) + 1;
		UINT HCLEN = bitStream.GetBits(4) + 4;

		// Read the Code Lengths, 3-bits each
		std::vector<UINT> CodeBitLengthCodes(CODE_LENGTHS_MAX_SIZE);
		for (int i = 0; i < CODE_LENGTHS_MAX_SIZE; i++)
		{
			CodeBitLengthCodes[CodeLengthsSwizzleTable[i]] = (i < HCLEN) ? bitStream.GetBits(3) : 0;
		}

		// Up to this point this is OK, I need to add tests for this
		// Build a Huffman tree based on those code lengths
		HuffmanTree testHuffmanTree;
		testHuffmanTree.BuildFromLengths(CodeBitLengthCodes);

		// Extract symbols for the Literal/Length Code Lengths
		while (literalLengthCodeLengths.size() < HLIT)
		{
			symbol_t symbol;
			ExtractSymbol(bitStream, testHuffmanTree, symbol);
			ProcessCodeLengthSymbol(bitStream, symbol, literalLengthCodeLengths);
		}

		// Extract symbols for the Distance Code Lengths
		while (distanceCodeLengths.size() < HDIST)
		{
			symbol_t symbol;
			ExtractSymbol(bitStream, testHuffmanTree, symbol);
			ProcessCodeLengthSymbol(bitStream, symbol, distanceCodeLengths);
		}
	}
	else
	{
		// STATIC HUFFMAN CODES
		literalLengthCodeLengths.resize(288);
		std::fill(literalLengthCodeLengths.begin(), literalLengthCodeLengths.end(), 8);
		std::fill_n(literalLengthCodeLengths.begin() + 144, 256 - 144, 9);
		std::fill_n(literalLengthCodeLengths.begin() + 256, 280 - 256, 7);
		distanceCodeLengths.resize(32);
		std::fill(distanceCodeLengths.begin(), distanceCodeLengths.end(), 5);
	}

	// Build the Literal/Length Huffman tree
	HuffmanTree literalLengthTree;
	literalLengthTree.BuildFromLengths(literalLengthCodeLengths);

	// Build the Distance Huffman tree
	HuffmanTree distanceTree;
	distanceTree.BuildFromLengths(distanceCodeLengths);

	// Decode symbols and perform the reverted LZSS algorithm, using Literal/Length and Distance Huffman trees
	LzssDecode(bitStream, literalLengthTree, distanceTree, symbolsList);
}

void ZLib::LzssDecode(BitStream& bitStream, HuffmanTree& literalLengthTree, HuffmanTree& distanceTree, std::vector<symbol_t>& symbolsList)
{
	UINT lzssIndex = 0;
	while (true)
	{
		symbol_t symbol;
		ExtractSymbol(bitStream, literalLengthTree, symbol);

		if (symbol < 256)
		{
#if ENABLE_PRINT
			std::cout << BOLDYELLOW << lzssIndex << " Literal symbol found: " << (UINT)symbol << RESET << std::endl;
#endif
			// Here we should be fine, this is a BYTE
			symbolsList.push_back(symbol);
		}
		else if (symbol == 256)
		{
#if ENABLE_PRINT
			std::cout << RED << "Found END_OF_BLOCK symbol\n" << RESET;
#endif
			break;
		}
		else if (symbol > 256)
		{
#if ENABLE_PRINT
			std::cout << BOLDCYAN << "Length symbol found: " << (UINT)symbol << RESET << std::endl;
#endif
			UINT lengthAdditionalBits = bitStream.GetBits(LiteralLengthAdditionalBits[symbol - 257]);
			UINT length = lengthAdditionalBits + LiteralLengthValueOffset[symbol - 257];
#if ENABLE_PRINT
			std::cout << BOLDCYAN << "\tcopy " << length << " times\n";
#endif

			// Here we need to read from the distance table
			symbol_t distanceSymbol;
			ExtractSymbol(bitStream, distanceTree, distanceSymbol);
			UINT distanceAdditionalBits = bitStream.GetBits(DistanceAdditionalBits[distanceSymbol]);
#if ENABLE_PRINT
			std::cout << GREEN << "Distance symbol found: " << (UINT)distanceSymbol << RESET << std::endl;
			std::cout << GREEN << "Need to read: " << (UINT)DistanceAdditionalBits[distanceSymbol] << RESET << std::endl;
			std::cout << GREEN << "Additional bits value: " << (UINT)distanceAdditionalBits << RESET << std::endl;
#endif
			UINT goBack = distanceAdditionalBits + DistanceValueOffset[distanceSymbol];
#if ENABLE_PRINT
			std::cout << GREEN << "\tgo back " << goBack << " places\n";
#endif

			const UINT N = symbolsList.size();
			for (int z = 0; z < length; z++)
			{
				// This also should be ok with the BYTE
				symbolsList.push_back(symbolsList[N - goBack + z]);
			}
		}
		lzssIndex++;
	}
#if ENABLE_PRINT
	std::cout << "Symbols found: " << lzssIndex << std::endl;
#endif
}

void ZLib::ExtractSymbol(BitStream& bitStream, HuffmanTree& testHuffmanTree, symbol_t& symbol)
{
	UINT currentCode = bitStream.GetBits(1);
	UINT currentSize = 1;
	while (AssetSuite::Result::OK != testHuffmanTree.GetSymbol(symbol, currentCode, currentSize))
	{
		// Here the code wasn't found, so we need another bit
		UINT nextBit = bitStream.GetBits(1);
		currentCode = currentCode << 1;
		currentCode = currentCode | nextBit;
		currentSize++;
	}
}

void ZLib::ProcessCodeLengthSymbol(BitStream& bitStream, symbol_t symbol, std::vector<UINT>& codeLengths)
{
	if ((symbol >= CodeLengthSymbols::LENGTH_0) && (symbol <= CodeLengthSymbols::LENGTH_15))
	{
		codeLengths.push_back(symbol);
		return;
	}

	UINT repeat = { 0 };
	symbol_t symbolToRepeat = { 0 };
	if (symbol == CodeLengthSymbols::COPY_PREVIOUS_CODE_LENGTH)
	{
		repeat = bitStream.GetBits(2) + 3;
		auto lastElementIndex = codeLengths.size() - 1;
		symbolToRepeat = codeLengths[lastElementIndex];
	}
	else if (symbol == CodeLengthSymbols::REPEAT_CODE_LENGTH_OF_0_FROM_3_TO_10_TIMES)
	{
		repeat = bitStream.GetBits(3) + 3;
	}
	else if (symbol == CodeLengthSymbols::REPEAT_CODE_LENGTH_OF_0_FROM_11_TO_138_TIMES)
	{
		repeat = bitStream.GetBits(7) + 11;
	}

	for (int i = 0; i < repeat; i++)
	{
		codeLengths.push_back(symbolToRepeat);
	}
}

BYTE ZLib::Char2Byte(CHAR c)
{
	return ((c >= '0') && (c <= '9')) ? c - 48 : c - 55;
}
