/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNodes.h"
#include "jitcat/GrammarBase.h"
#include "jitcat/CatGrammarType.h"


namespace jitcat::Grammar
{
	//This class defines the grammar for the JitCat language.
	//It can also do partial grammars like a grammar for just a JitCat expression.
	//The available types of grammars are defined by the CatGrammarType.
	//GrammarBase contains all the plumbing for creating grammars, CatGrammar contains only the grammar specific to JitCat.
	//Grammars are defined by a production rule system. Each production has a semantic action associated with it in the form of a static member function of this class.
	class CatGrammar: public GrammarBase
	{
		//Productions
		enum class Prod
		{
			Root,
			Identifier,
			SourceFile,
			Definitions,
			Definition,
			ClassDefinition,
			ClassContents,
			Declaration,
			InheritanceDefinition,
			FunctionDefinition,
			FunctionParameters,
			FunctionParameterDefinitions,
			VariableDeclaration,
			VariableDefinition,
			VariableInitialization,
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
			Return,
			Statement,
			ScopeBlock,
			ScopeBlockStatements
		};
	public: 
		CatGrammar(Tokenizer::TokenizerBase* tokenizer, CatGrammarType grammarType);
		virtual const char* getProductionName(int production) const;

	private:
		//Semantic action
		static AST::ASTNode* pass(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* link(const Parser::ASTNodeParser& nodeParser);

		static AST::ASTNode* sourceFile(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* classDefinition(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* inheritanceDefinition(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* functionDefinition(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* functionParameterDefinitions(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* variableDeclaration(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* variableDefinition(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* typeName(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* ifStatement(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* returnStatement(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* scopeBlock(const Parser::ASTNodeParser& nodeParser);

		static AST::ASTNode* assignmentOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* infixOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* prefixOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* operatorNew(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* literalToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* identifierToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* argumentListToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* functionCallToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* memberAccessToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* memberFunctionCallToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* arrayIndexToken(const Parser::ASTNodeParser& nodeParser);
	};

} //End namespace jitcat::Grammar