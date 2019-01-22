/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatASTNodes.h"
#include "Grammar.h"

class CatGrammar: public Grammar
{
	//Productions
	enum class Prod
	{
		Root,
		ClassDefinition,
		ClassContents,
		Declaration,
		FunctionDefinition,
		FunctionParameterDefinition,
		FunctionParameterDefRepeat,
		VariableDeclaration,
		VariableDeclarationTail,
		OperatorP2,
		OperatorP3,
		OperatorP4,
		OperatorP5,
		OperatorP6,
		OperatorP7,
		OperatorP8,
		OperatorP9,
		OperatorP10,
		OperatorP11,
		Expression,
		ExpressionBlock,
		ExpressionBlockContents,
		FlowControl,
		IfThen,
		Else,
		ElseBody,
		Type,
		FunctionCall,
		FunctionCallArguments,
		FunctionCallArgumentRepeat,
		Literal,
		Assignment,
		ObjectMemberAccess,
		ObjectMemberAccessAction,
		Return
	};
public: 
	CatGrammar(Tokenizer* tokenizer);
	virtual const char* getProductionName(int production) const;
	static bool isTypedExpression(CatASTNodeType node);

private:
	//Semantic action
	static ASTNode* pass(const ASTNodeParser& nodeParser);
	static ASTNode* link(const ASTNodeParser& nodeParser);
	static ASTNode* assignmentOperator(const ASTNodeParser& nodeParser);
	static ASTNode* infixOperator(const ASTNodeParser& nodeParser);
	static ASTNode* prefixOperator(const ASTNodeParser& nodeParser);
	static ASTNode* literalToken(const ASTNodeParser& nodeParser);
	static ASTNode* identifierToken(const ASTNodeParser& nodeParser);
	static ASTNode* argumentListToken(const ASTNodeParser& nodeParser);
	static ASTNode* functionCallToken(const ASTNodeParser& nodeParser);
	static ASTNode* memberAccessToken(const ASTNodeParser& nodeParser);
	static ASTNode* memberFunctionCallToken(const ASTNodeParser& nodeParser);
	static ASTNode* arrayIndexToken(const ASTNodeParser& nodeParser);
};