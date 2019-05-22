/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatRuntimeContext;
}
#include "jitcat/CatStatement.h"
#include "jitcat/CatGenericType.h"

#include <any>

namespace jitcat::AST
{

	class CatExpression: public CatStatement
	{
	public:
		CatExpression(const Tokenizer::Lexeme& lexeme): CatStatement(lexeme) {}
		CatExpression(const CatExpression& other): CatStatement(other) {}
	};


} //End namespace jitcat::AST