/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class RuntimeContext;
	namespace AST
	{
		class ASTNode;
	}
	namespace Tokenizer
	{
		class ParseToken;
	}
}

#include "jitcat/Lexeme.h"
#include <vector>

namespace jitcat::Parser
{
	class StackItem;

	class ASTNodeParser
	{
	public:
		ASTNodeParser(const std::vector<Parser::StackItem*>& stack, std::size_t numItems, RuntimeContext* context);
		int getNumItems() const;
		Parser::StackItem* getItem(unsigned int index) const;
		AST::ASTNode* getASTNodeByIndex(unsigned int index) const;
		template<typename ASTNodeT>
		ASTNodeT* getASTNodeByIndex(unsigned int index) const;
		const Tokenizer::ParseToken* getTerminalByIndex(unsigned int index) const;
		RuntimeContext* getContext() const;
		Tokenizer::Lexeme getStackLexeme() const;

	private:
		const std::vector<Parser::StackItem*>& stack;
		std::size_t numItems;
		std::size_t startIndex;

		//not owned
		RuntimeContext* context;
	};

	template<typename ASTNodeT>
	inline ASTNodeT* ASTNodeParser::getASTNodeByIndex(unsigned int index) const
	{
		return static_cast<ASTNodeT*>(getASTNodeByIndex(index));
	}

} //End namespace jitcat::AST