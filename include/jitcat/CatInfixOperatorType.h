/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::AST
{

	enum class CatInfixOperatorType
	{
		Plus,
		Minus,
		Multiply,
		Divide,
		Modulo,
		Greater,
		Smaller,
		GreaterOrEqual,
		SmallerOrEqual,
		Equals,
		NotEquals,
		LogicalAnd,
		LogicalOr,
		Count
	};


	const char* toString(CatInfixOperatorType infixOperator);

} //End namespace jitcat::AST