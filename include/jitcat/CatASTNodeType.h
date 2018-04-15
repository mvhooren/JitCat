/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


enum class CatASTNodeType
{
	Literal,
	Identifier,
	InfixOperator,
	PrefixOperator,
	ParameterList,
	FunctionCall,
	LinkedList,
	MemberAccess,
	ArrayIndex,
	MemberFunctionCall,
	ScopeRoot
};
