/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::AST
{
	class ASTNode;
}
namespace jitcat::Tokenizer
{
	class ParseToken;
}
namespace jitcat::Grammar
{
	class Production;
}

#include <stddef.h>


namespace jitcat::Parser
{
	class DFAState;


	class StackItem
	{
	public:
		StackItem():
			state(nullptr),
			astNode(nullptr)
		{}
		virtual ~StackItem() {}
		DFAState* state;
		AST::ASTNode* astNode;
		virtual const Tokenizer::ParseToken* getTokenIfToken() const {return 0;}
		virtual const Grammar::Production* getProductionIfProduction() const {return 0;}
	};

} //End namespace jitcat::Parser