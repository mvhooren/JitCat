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
		ArrayIndex,
		AssignmentOperator,
		ClassDefinition,
		ForLoop,
		FunctionCall,
		FunctionDefinition,
		FunctionParameterDefinitions,
		Identifier,
		IfStatement,
		InfixOperator,
		InheritanceDefinition,
		LinkedList,
		Literal,
		MemberAccess,
		MemberFunctionCall,
		OperatorNew,
		OperatorNewArray,
		ParameterList,
		PrefixOperator,
		Range,
		ReturnStatement,
		ScopeBlock,
		ScopeRoot,
		SourceFile,
		StaticIdentifier,
		TypeName,
		VariableDeclaration,
		VariableDefinition
	};

} //End namespace jitcat::AST