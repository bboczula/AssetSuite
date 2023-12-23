#pragma once

#include <string>
#include <Windows.h>

#include "../bitstream/BitStream.h"
#include "Huffman.h"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

enum CodeLengthSymbols
{
	LENGTH_0,
	LENGTH_1,
	LENGTH_2,
	LENGTH_3,
	LENGTH_4,
	LENGTH_5,
	LENGTH_6,
	LENGTH_7,
	LENGTH_8,
	LENGTH_9,
	LENGTH_10,
	LENGTH_11,
	LENGTH_12,
	LENGTH_13,
	LENGTH_14,
	LENGTH_15,
	COPY_PREVIOUS_CODE_LENGTH,
	REPEAT_CODE_LENGTH_OF_0_FROM_3_TO_10_TIMES,
	REPEAT_CODE_LENGTH_OF_0_FROM_11_TO_138_TIMES,
	COUNT
};

class ZLib
{
public:
	std::vector<BYTE> Decode(const std::string& byteString);
	std::vector<BYTE> Decode(const BYTE* data, size_t size);
	void Inflate(BitStream& bitStream, std::vector<symbol_t>& symbolsList);
	std::vector<BYTE> Decode(const std::vector<BYTE>& byteStream);
private:
	void HandleCompressedBlock(const UINT& BTYPE, BitStream& bitStream, std::vector<symbol_t>& symbolsList);
	void LzssDecode(BitStream& bitStream, HuffmanTree& literalLengthTree, HuffmanTree& distanceTree, std::vector<symbol_t>& symbolsList);
	void ExtractSymbol(BitStream& bitStream, HuffmanTree& testHuffmanTree, symbol_t& symbol);
	void ProcessCodeLengthSymbol(BitStream& bitStream, symbol_t symbol, std::vector<UINT>& codeLengths);
	BYTE Char2Byte(CHAR c);
};