/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"
#include "jitcat/TypeOwnershipSemantics.h"

namespace jitcat::AST
{
	class CatTypedExpression;

	class CatOwnershipSemanticsNode: public CatASTNode
	{
	public:
		CatOwnershipSemanticsNode(Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme);

		CatOwnershipSemanticsNode(const CatOwnershipSemanticsNode& other);
		virtual ~CatOwnershipSemanticsNode();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		Reflection::TypeOwnershipSemantics getOwnershipSemantics() const;

	private:
		Reflection::TypeOwnershipSemantics ownershipSemantics;
	};
}
