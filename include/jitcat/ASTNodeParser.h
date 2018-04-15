/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class ASTNode;
class RuntimeContext;
class StackItem;
class ParseToken;

#include <vector>


class ASTNodeParser
{
public:
	ASTNodeParser(const std::vector<StackItem*>& stack, std::size_t numItems, RuntimeContext* context);
	int getNumItems() const;
	StackItem* getItem(unsigned int index) const;
	ASTNode* getASTNodeByIndex(unsigned int index) const;
	const ParseToken* getTerminalByIndex(unsigned int index) const;
	RuntimeContext* getContext() const;

private:
	const std::vector<StackItem*>& stack;
	std::size_t numItems;
	std::size_t startIndex;

	//not owned
	RuntimeContext* context;
};