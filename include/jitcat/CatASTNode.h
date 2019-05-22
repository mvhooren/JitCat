/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatRuntimeContext;
	class ExpressionErrorManager;
}
#include "jitcat/ASTNode.h"
#include "jitcat/CatASTNodeType.h"

namespace jitcat::AST
{


	class CatASTNode: public ASTNode
	{
	public:
		CatASTNode(const Tokenizer::Lexeme& lexeme): ASTNode(lexeme) {}
		CatASTNode(const CatASTNode& other): ASTNode(other) {}

		virtual CatASTNode* copy() const = 0;
		virtual void print() const = 0;
		virtual CatASTNodeType getNodeType() const = 0;
	};


} //End namespace jitcat::AST
