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
	//It can also do partial grammars, for example CatGrammarType::Expression generates 
	//a grammar that can only handle expressions.
	//The available types of grammars are defined by the CatGrammarType.
	//GrammarBase contains all the plumbing for creating grammars, CatGrammar contains only the grammar specific to JitCat.
	//Grammars are defined by a production rule system. Each production has a semantic action associated with it in the form of a static member function of this class.
	class CatGrammar: public GrammarBase
	{
		//Productions
		enum class Prod: unsigned short
		{
			Root,
			TypeOrIdentifier,
			StaticAccessor,
			StaticIdentifier,
			StaticFunctionCall,
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
			ForLoop,
			Range,
			Continue,
			Break,
			OwnershipSemantics,
			FunctionOrConstructorCall,
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
		//The constructor takes a tokenizer and the type of grammar that is to be defined.
		//The tokenizer defines all the tokens for the language, such as literals, identifiers and operators.
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
		static AST::ASTNode* arrayTypeName(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* typeName(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* nestedTypeName(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* basicTypeName(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* ownershipSemantics(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* ifStatement(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* returnStatement(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* scopeBlock(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* forLoop(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* range(const Parser::ASTNodeParser& nodeParser);
		
		static AST::ASTNode* assignmentOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* infixOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* prefixOperator(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* toOperatorNew(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* toOperatorNewArray(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* literalToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* argumentListToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* functionCallToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* memberAccessToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* memberFunctionCallToken(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* arrayIndexToken(const Parser::ASTNodeParser& nodeParser);

		static AST::ASTNode* toIdentifier(const Parser::ASTNodeParser& nodeParser);
		static AST::ASTNode* toFunctionCall(const Parser::ASTNodeParser& nodeParser);
	
	private:
		//Tokens types:
		static unsigned short comment;
		static unsigned short ws;
		static unsigned short lit;
		static unsigned short id;
		static unsigned short err;
		static unsigned short one;
		static unsigned short two;
		static unsigned short eof;
	};

} //End namespace jitcat::Grammar