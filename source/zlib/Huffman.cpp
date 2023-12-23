#include <algorithm>
#include <cassert>
#include <iomanip>
#include <numeric>

#include "Huffman.h"

#define ENABLE_PRINT 0

AssetSuite::Result HuffmanTree::GetSymbol(symbol_t& symbol, UINT code, UINT codeLength)
{
	if (!codeLength)
	{
		symbol = '?';
		return AssetSuite::Result::SymbolNotUsed;
	}

	if (!codeTable.size())
	{
		symbol = '?';
		return AssetSuite::Result::CodeTableIsEmpty;
	}

	for (auto it = codeTable.begin(); it != codeTable.end(); it++)
	{
		if ((it->second.length == codeLength) && (it->second.code == code))
		{
			symbol = it->first;
			return AssetSuite::Result::OK;
		}
	}
	symbol = '?';
	return AssetSuite::Result::SymbolNotFound;
}

void HuffmanTree::BuildFromLengths(const std::vector<UINT>& codeLengths)
{
#if ENABLE_PRINT
	std::cout << "BuildFromLengths" << std::endl;
	for (int i = 0; i < codeLengths.size(); i++)
	{
		std::cout << i << ") " << codeLengths[i] << std::endl;
	}
#endif
	assert(codeTable.empty());
	PopulateCodeTable(codeLengths);
	GenerateCanonicalCodeTable();
	GenerateTreeFromCodeTable();
	PrintCodeTable();
	PrintTree();
}

void HuffmanTree::BuildFromLengths(const SymbolList& symbols, const std::vector<UINT>& codeLengths)
{
	assert(codeTable.empty());
	PopulateCodeTable(symbols, codeLengths);
	GenerateCanonicalCodeTable();
	GenerateTreeFromCodeTable();
	PrintCodeTable();
	PrintTree();
}

void HuffmanTree::BuildFromFrequencies(const std::vector<UINT>& frequencies)
{
	assert(codeTable.empty());
	SymbolList symbols(frequencies.size());
	std::iota(symbols.begin(), symbols.end(), 0);
	BuildFromFrequencies(symbols, frequencies);
}

void HuffmanTree::BuildFromFrequencies(const SymbolList& symbols, const LengthList& frequencies)
{
	assert(codeTable.empty());
	GenerateTreeFromFrequencies(symbols, frequencies);
	TraverseTreeAndGenerateCodes(heap.top(), 0, 0);
	GenerateCanonicalCodeTable();
	PrintCodeTable();
}

void HuffmanTree::Reset()
{
	Traverse(heap.top(), [](HuffmanNode* node) -> void
		{
			if (node)
			{
				delete node;
			}
		});
	heap.pop();
	assert(heap.size() == 0);

	codeTable.clear();
	assert(codeTable.size() == 0);
}

void HuffmanTree::Traverse(HuffmanNode* node, std::function<void(HuffmanNode* node)> function)
{
	if (node == nullptr)
	{
		return;
	}

	Traverse(node->left, function);
	function(node);
	Traverse(node->right, function);
}

void HuffmanTree::PrintTree()
{
	if (heap.empty())
	{
		return;
	}

	Traverse(heap.top(), [](HuffmanNode* node) -> void
		{
			if (node->hasSymbol)
			{
#if ENABLE_PRINT
				std::cout << "node: " << (UINT)node->symbol << "\n";
#endif
			}
		});
}

void HuffmanTree::PrintCodeTable()
{
#if ENABLE_PRINT
	std::cout << "+---------------------------------+\n";
	std::cout << "| Here's the CANONICAL Code Table |\n";
	std::cout << "+--------+------+--------+--------+\n";
	std::cout << "| SYMBOL | CODE | LENGTH |  BITS  |\n";
	std::cout << "+--------+------+--------+--------+\n";

	for (auto it = codeTable.begin(); it != codeTable.end(); it++)
	{
		std::bitset<15> bits(it->second.code);
		std::string bitString = bits.to_string();
		if (it->second.length)
		{
			bitString.erase(0, 15 - it->second.length);
		}
		else
		{
			bitString = "-";
		}
		std::cout << "| " << std::setw(6) << std::setfill(' ') << (UINT)it->first;
		std::cout << " | " << std::setw(4) << std::setfill(' ') << (UINT)it->second.code;
		std::cout << " | " << std::setw(6) << std::setfill(' ') << it->second.length;
		std::cout << " | " << std::setw(6) << std::setfill(' ') << bitString << " |\n";
	}
	std::cout << "+--------+------+--------+--------+\n";
#endif
}

void PrintLine(UINT leftPadding, CHAR symbol)
{
	if (leftPadding)
	{
		std::cout << std::setw(leftPadding) << std::setfill(' ') << ' ';
	}
	std::cout << symbol;
}

void PrintDoubleLine(UINT padding, CHAR symbol)
{
	if (padding)
	{
		std::cout << std::setw(padding) << std::setfill(' ') << ' ';
	}
	std::cout << symbol;
	if (padding)
	{
		std::cout << std::setw(padding) << std::setfill(' ') << ' ';
	}
}

void PrintDoubleLine(UINT leftPadding, CHAR symbol, UINT rightPadding)
{
	if (leftPadding - 1)
	{
		std::cout << std::setw(leftPadding - 1) << std::setfill(' ') << ' ';
	}
	std::cout << symbol;
	if (rightPadding)
	{
		std::cout << std::setw(rightPadding) << std::setfill(' ') << ' ';
	}
}

void PrintLine(UINT margin, CHAR s1, UINT leafPadding, CHAR s2, UINT rightPadding)
{
	if (margin)
	{
		std::cout << std::setw(margin) << std::setfill(' ') << ' ';
	}
	std::cout << s1 << std::setw(leafPadding) << std::setfill(' ') << ' ' << s2 << std::setw(rightPadding) << std::setfill(' ') << ' ';
}

void HuffmanTree::PrintPrettyTree()
{
	const UINT LEAF_PADDING = 5;
	const UINT BRANCH_PADDING = 3;
	const UINT NUM_OF_LEVELS = 4;
	const UINT MAX_WIDTH = std::pow(2, NUM_OF_LEVELS - 1)
		+ ((std::pow(2, NUM_OF_LEVELS - 1) / 2) * LEAF_PADDING)
		+ (((std::pow(2, NUM_OF_LEVELS - 1) / 2) - 1) * BRANCH_PADDING);
#if ENABLE_PRINT
	std::cout << "MAX_WIDTH: " << MAX_WIDTH << std::endl;
#endif
	for (int i = 0; i < NUM_OF_LEVELS; i++)
	{
		UINT N = floor(1.0f / (float)std::pow(2, i + 1) * MAX_WIDTH);
		//std::cout << N << ": ";
		for (int j = 0; j < std::pow(2, i); j++)
		{
			if (i != NUM_OF_LEVELS - 1)
			{
				PrintDoubleLine(N, 'X', N + 2);
			}
			else
			{
				if (j == 0)
				{
					PrintLine(0, 'x');
				}
				else if (j % 2 == 1)
				{
					PrintLine(LEAF_PADDING, 'x');
				}
				else
				{
					PrintLine(BRANCH_PADDING, 'y');
				}
			}
		}
		std::cout << std::endl;
	}
}

void HuffmanTree::GenerateCanonicalCodeTable()
{
	std::sort(codeTable.begin(), codeTable.end(), CodeLengthAndLexiLessThan());

	UINT currentValue, prevBitLength;
	currentValue = 0;
	prevBitLength = codeTable[0].first;
	for (auto it = codeTable.begin(); it != codeTable.end(); it++)
	{
		if (it->second.length)
		{
			UINT shiftBits = it->second.length - prevBitLength;
			currentValue = currentValue << shiftBits;
			it->second.code = currentValue;
			currentValue++;
			prevBitLength = it->second.length;
		}
	}
}

void HuffmanTree::GenerateTreeFromFrequencies(const SymbolList& symbols, const std::vector<UINT> frequencies)
{
	assert(symbols.size() == frequencies.size());

	for (int i = 0; i < frequencies.size(); i++)
	{
		heap.push(new HuffmanNode(symbols[i], nullptr, nullptr, frequencies[i]));
	}

	do
	{
		HuffmanNode* last = heap.top();
		heap.pop();
		HuffmanNode* nextToLast = heap.top();
		heap.pop();

		heap.push(new HuffmanNode(last, nextToLast, last->accumulatedFrequency + nextToLast->accumulatedFrequency));
	} while (heap.size() > 1);
}

void HuffmanTree::PopulateCodeTable(const std::vector<UINT> codeLengths)
{
	SymbolList symbols(codeLengths.size());
	std::iota(symbols.begin(), symbols.end(), 0);
	PopulateCodeTable(symbols, codeLengths);
}

void HuffmanTree::PopulateCodeTable(const SymbolList& symbols, const std::vector<UINT> codeLengths)
{
	assert(symbols.size() == codeLengths.size());

	for (int i = 0; i < codeLengths.size(); i++)
	{
		if (codeLengths[i])
		{
			AssetSuite::HuffmanCode code = { 0, codeLengths[i] };
			codeTable.push_back(std::make_pair(symbols[i], code));
		}
	}
}

void HuffmanTree::GenerateTreeFromCodeTable()
{
	if (heap.size() == 0)
	{
		heap.push(new HuffmanNode(nullptr, nullptr, 0));
	}

	// Here you can assume that the code table is correctly filled
	// Basically you need to create leaves for each symbol
	HuffmanNode* currentNode = nullptr;
	for (auto it = codeTable.begin(); it != codeTable.end(); it++)
	{
		// For each node start from the top
		currentNode = heap.top();

#if ENABLE_PRINT
		std::cout << it->first << ") " << it->second.code << " - length " << it->second.length << ", top: " << currentNode << std::endl;
#endif
		// If I know the code length, I know how many branches I need to make in order to make a new node
		for (int i = 0; i < it->second.length; i++)
		{
			UINT currentBit = (it->second.code & (0x1 << i)) >> i;
#if ENABLE_PRINT
			std::cout << "\tcurrentBit: " << currentBit << std::endl;
#endif
			HuffmanNode*& nextNode = currentBit ? currentNode->right : currentNode->left;
			if (!nextNode)
			{
				nextNode = new HuffmanNode(nullptr, nullptr, 0);
			}
			currentNode = nextNode;
		}
		// Here, the current node should be ok, this should be your leaf;
		currentNode->hasSymbol = true;
		currentNode->left = nullptr;
		currentNode->right = nullptr;
		currentNode->symbol = it->first;
	}
}

void HuffmanTree::TraverseTreeAndGenerateCodes(HuffmanNode* node, UINT code, UINT codeLength)
{
	if (node == nullptr)
	{
		return;
	}

	TraverseTreeAndGenerateCodes(node->left, code << 1, codeLength + 1);
	if (node->hasSymbol)
	{
		codeTable.push_back({ node->symbol, { code, codeLength } });
	}
	TraverseTreeAndGenerateCodes(node->right, (code << 1) | 0x1, codeLength + 1);
}