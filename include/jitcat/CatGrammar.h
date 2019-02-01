/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNodes.h"
#include "jitcat/GrammarBase.h"

namespace jitcat::Grammar
{

	class CatGrammar: public GrammarBase
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
		CatGrammar(Tokenizer::TokenizerBase* tokenizer);
		virtual const char* getProductionName(int production) const;
		static bool isTypedExpression(AST::CatASTNodeType node);

	private:
		//Semantic action
		static AST::ASTNode* pass(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* link(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* assignmentOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* infixOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* prefixOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* literalToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* identifierToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* argumentListToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* functionCallToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* memberAccessToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* memberFunctionCallToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* arrayIndexToken(const Parser::ASTNodeParser& nodeParser);
	};

} //End namespace jitcat::Grammar