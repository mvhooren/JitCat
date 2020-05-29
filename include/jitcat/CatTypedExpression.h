/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatStatement.h"
#include "jitcat/CatGenericType.h"

namespace jitcat::AST
{

	class CatTypedExpression: public CatStatement
	{
	public:
		CatTypedExpression(const Tokenizer::Lexeme& lexeme): CatStatement(lexeme) {}
		CatTypedExpression(const CatTypedExpression& other): CatStatement(other) {}

		virtual const CatGenericType& getType() const = 0;
		virtual bool isConst() const = 0;
		virtual bool isAssignable() const {return false;}
		virtual bool isTypedExpression() const override final { return true;}
	};

} //End namespace jitcat::AST