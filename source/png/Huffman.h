#pragma once

#include <string>
#include <bitset>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <queue>
#include <functional>

/*
* The HuffmanTree class allows you to create Huffman Tree and calculate Huffman Codes. The class
* contains two structures that store the data, one is the priority_queue that implements the binary
* tree and the other is a lookup Code Table, that could technically allow for faster symbol
* resolve.
*
* The class contains a variety of Build() functions, each taking different set of input arguments,
* along with the GetSymbol() function that returns a functions for given code and/or code length,
* and finally the Traverse function that would allow for a binary tree traversal and running
* user's own, custom function on the data. There are also some minor helper functions, like
* Print() or Size(). There is also a Reset() function for manual resetting.
*
* Whenever you run the Build() function, both priority_queue and the lookup table will be regenerated.
* The Reset() function will manually be called whenever you call any of the Build() functions.
*
* INTERFACE
* There are two main ways to build the Huffman Tree, the first is with the code lengths and second is
* from symbol frequencies. In both cases you can either provide the alphabet (which is a list of symbols
* in order) or not, in which case the symbols would be simply assigned starting from 0x01.
*/

// If this was BYTE, we could have had an error, you really have to be careful for thos casts!!!!!
// It would be also great to understand why is this happening, like WTF actually
using symbol_t = SHORT;
using LengthList = std::vector<UINT>;
using SymbolList = std::vector<symbol_t>;

struct HuffmanNode
{
	HuffmanNode(HuffmanNode* left, HuffmanNode* right, const float accumulatedFrequency)
		: symbol('$'), left(left), right(right), accumulatedFrequency(accumulatedFrequency), hasSymbol(false)
	{
	}
	HuffmanNode(const CHAR& symbol, HuffmanNode* left, HuffmanNode* right, const float accumulatedFrequency)
		: symbol(symbol), left(nullptr), right(nullptr), accumulatedFrequency(accumulatedFrequency), hasSymbol(true)
	{
	}
	BOOL hasSymbol;
	HuffmanNode* left;
	HuffmanNode* right;
	const float accumulatedFrequency;
	symbol_t symbol;
};

namespace AssetSuite
{
	enum Result
	{
		OK,
		SymbolNotFound,
		SymbolNotUsed,
		CodeTableIsEmpty
	};

	struct HuffmanCode
	{
		UINT code;
		UINT length;
	};
}

class HuffmanTree
{
public:
	/// <summary>
	/// Returns the symbol from the alphabet with given code and code length. If the code length is zero, then the symbol is not used.
	/// </summary>
	/// <param name="symbol">Reference to a variable, where the found symbol would be put.</param>
	/// <param name="code">A code to find, presented as unsigned integer</param>
	/// <param name="codeLength">A code length of the symbol</param>
	/// <returns>A Result::OK if symbol was found, and error in other case</returns>
	AssetSuite::Result GetSymbol(symbol_t& symbol, UINT code, UINT codeLength);

	// We could have a GetSymbol function that takes different arguments, like without the code length, and maybe traverse the tree
	// I wonder though, would that have to be a string? With MaxLength maybe?

	/// <summary>
	/// Builds a Huffman Tree from the list of code bit-lengths.
	/// </summary>
	/// <param name="codeLengths">a list of bit lengths for each corresponding code</param>
	void BuildFromLengths(const std::vector<UINT>& codeLengths);

	void BuildFromLengths(const SymbolList& symbols, const std::vector<UINT>& codeLengths);

	/// <summary>
	/// Builds a Huffman Tree from the list of symbols and the corresponding symbols frequencies
	/// </summary>
	/// <param name="symbols">A list of symbols from the alphabet</param>
	/// <param name="frequencies">A list of frequecies for each symbol</param>
	void BuildFromFrequencies(const SymbolList& symbols, const LengthList& frequencies);

	void BuildFromFrequencies(const std::vector<UINT>& frequencies);

	/// <summary>
	/// Resets all the data related to the tree, including memory heap.
	/// </summary>
	void Reset();

	void Traverse(HuffmanNode* node, std::function<void(HuffmanNode* node)> function);

	void PrintTree();

	void PrintCodeTable();

	void PrintPrettyTree();
private:
	void GenerateTreeFromFrequencies(const SymbolList& codeNames, const std::vector<UINT> frequencies);
	void GenerateTreeFromCodeTable();
	void PopulateCodeTable(const std::vector<UINT> codeLengths);
	void PopulateCodeTable(const SymbolList& symbols, const std::vector<UINT> codeLengths);
	void GenerateCanonicalCodeTable();
	void TraverseTreeAndGenerateCodes(HuffmanNode* node, UINT code, UINT codeLength);
	struct FrequencyGreaterThan
	{
		bool operator()(HuffmanNode const* a, HuffmanNode const* b) const
		{
			return a->accumulatedFrequency > b->accumulatedFrequency;
		}
	};
	struct CodeLengthAndLexiLessThan
	{
		bool operator()(std::pair<symbol_t, AssetSuite::HuffmanCode> a, std::pair<symbol_t, AssetSuite::HuffmanCode> b) const
		{
			if (a.second.length < b.second.length)
			{
				return true;
			}
			else if (a.second.length == b.second.length)
			{
				if (a.first < b.first)
				{
					return true;
				}
			}
			return false;
		}
	};
	std::priority_queue < HuffmanNode*, std::vector<HuffmanNode*>, FrequencyGreaterThan> heap;
	std::vector<std::pair<symbol_t, AssetSuite::HuffmanCode>> codeTable;
};