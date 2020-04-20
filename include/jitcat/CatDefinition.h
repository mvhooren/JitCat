/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"

#include <optional>


namespace jitcat::AST
{
	class CatDefinition: public CatASTNode
	{
	public:
		CatDefinition(const Tokenizer::Lexeme& lexeme): CatASTNode(lexeme) {};
		CatDefinition(const CatDefinition& other): CatASTNode(other) {}

		virtual ~CatDefinition() {};
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) = 0;
	};

};
