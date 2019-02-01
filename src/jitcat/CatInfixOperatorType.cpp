/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatInfixOperatorType.h"

using namespace jitcat::AST;


const char* jitcat::AST::toString(CatInfixOperatorType infixOperator)
{
	switch (infixOperator)
	{
		default:									return "none";
		case CatInfixOperatorType::Plus:			return "+";	
		case CatInfixOperatorType::Minus:			return "-";
		case CatInfixOperatorType::Multiply:		return "*";
		case CatInfixOperatorType::Divide:			return "/";
		case CatInfixOperatorType::Modulo:			return "%";
		case CatInfixOperatorType::Greater:			return ">";
		case CatInfixOperatorType::Smaller:			return "<";
		case CatInfixOperatorType::GreaterOrEqual:	return ">=";
		case CatInfixOperatorType::SmallerOrEqual:	return "<=";
		case CatInfixOperatorType::Equals:			return "==";
		case CatInfixOperatorType::NotEquals:		return "!=";
		case CatInfixOperatorType::LogicalAnd:		return "&&";
		case CatInfixOperatorType::LogicalOr:		return "||";
	}
}
