/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ASTNodeParser.h"
#include "jitcat/ASTNode.h"
#include "jitcat/Production.h"
#include "jitcat/StackItem.h"
#include <cassert>
using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Parser;
using namespace jitcat::Tokenizer;


ASTNodeParser::ASTNodeParser(const std::vector<StackItem*>& stack, std::size_t numItems, RuntimeContext* context):
	stack(stack),
	numItems(numItems),
	context(context)
{
	if (stack.size() >= numItems)
	{
		startIndex = stack.size() - numItems;
	}
	else
	{
		startIndex = stack.size();
	}
}


int ASTNodeParser::getNumItems() const
{
	return (int)numItems;
}


StackItem* ASTNodeParser::getItem(unsigned int index) const
{
	if (startIndex + index < stack.size())
	{
		return stack[startIndex + index];
	}
	else
	{
		return nullptr;
	}
}


ASTNode* ASTNodeParser::getASTNodeByIndex(unsigned int index) const
{
	std::size_t currentIndex = 0;
	for (std::size_t i = startIndex; i < stack.size(); i++)
	{
		if (stack[i]->astNode != nullptr)
		{
			if (index == currentIndex)
			{
				return stack[i]->astNode;
			}
			currentIndex++;
		}
	}
	return nullptr;
}


const ParseToken* ASTNodeParser::getTerminalByIndex(unsigned int index) const
{
	std::size_t currentIndex = 0;
	for (std::size_t i = startIndex; i < stack.size(); i++)
	{
		if (stack[i]->getTokenIfToken() != nullptr)
		{
			if (index == currentIndex)
			{
				return stack[i]->getTokenIfToken();
			}
			currentIndex++;
		}
	}
	return nullptr;
}


RuntimeContext* ASTNodeParser::getContext() const
{
	return context;
}


Tokenizer::Lexeme jitcat::Parser::ASTNodeParser::getStackLexeme() const
{
	const char* start = nullptr;
	const char* end = nullptr;
	for (std::size_t i = startIndex; i < stack.size(); i++)
	{
		const char* itemStart = nullptr;
		const char* itemEnd = nullptr;
		if (stack[i]->astNode != nullptr)
		{
			itemStart = stack[i]->astNode->getLexeme().data();
			itemEnd = itemStart + stack[i]->astNode->getLexeme().length();
		}
		else if (stack[i]->getTokenIfToken() != nullptr)
		{
			const Lexeme& tokenLexeme = stack[i]->getTokenIfToken()->getLexeme();
			itemStart = tokenLexeme.data();
			itemEnd = itemStart + tokenLexeme.length();
		}
		if (start == nullptr || start > itemStart)
		{
			start = itemStart;
		}
		if (end == nullptr || end < itemEnd)
		{
			end = itemEnd;
		}
	}
	return Lexeme(start, end - start);
}
