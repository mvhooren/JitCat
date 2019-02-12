/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::AST
{

	enum class CatASTNodeType
	{
		SourceFile,
		ClassDefinition,
		FunctionDefinition,
		FunctionParameterDefinitions,
		TypeName,
		Literal,
		Identifier,
		InfixOperator,
		AssignmentOperator,
		PrefixOperator,
		ParameterList,
		FunctionCall,
		LinkedList,
		MemberAccess,
		ArrayIndex,
		MemberFunctionCall,
		ScopeRoot,
		VariableDeclaration,
		IfStatement,
		ReturnStatement,
		ScopeBlock
	};

} //End namespace jitcat::AST