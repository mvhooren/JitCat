/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"

#include <memory>

namespace jitcat::AST
{

	class CatLinkNode: public CatASTNode
	{
	public:
		CatLinkNode(CatASTNode* me, CatASTNode* next, const jitcat::Tokenizer::Lexeme& lexeme);
		CatLinkNode(const CatLinkNode&) = delete;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

		std::unique_ptr<CatASTNode> me;
		std::unique_ptr<CatASTNode> next;
	};

} // End namespace jitcat::AST