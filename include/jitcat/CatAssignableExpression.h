/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"

#include <optional>
#include <string>

namespace jitcat::AST
{

	class CatAssignableExpression: public CatTypedExpression
	{
	public:
		CatAssignableExpression(const Tokenizer::Lexeme& lexeme): CatTypedExpression(lexeme) {}
		CatAssignableExpression(const CatAssignableExpression& other): CatTypedExpression(other) {}

		virtual bool isAssignable() const override {return true;}
		virtual const CatGenericType& getAssignableType() const = 0;
		virtual std::any executeAssignable(CatRuntimeContext* runtimeContext) = 0;
		virtual std::optional<std::string> getAssignableVariableName() const { return std::optional<std::string>();}
	};


} //End namespace jitcat::AST