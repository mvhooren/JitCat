/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatGrammar.h"
#include "jitcat/ASTNodeParser.h"
#include "jitcat/CatASTNodes.h"
#include "jitcat/CatScopeRoot.h"
#include "jitcat/CatTokenIds.h"
#include "jitcat/ConstantToken.h"
#include "jitcat/Lexeme.h"
#include "jitcat/OneCharToken.h"
#include "jitcat/Tools.h"

#include <stdlib.h>
#include <type_traits>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Cat;
using namespace jitcat::Grammar;
using namespace jitcat::Parser;
using namespace jitcat::Reflection;
using namespace jitcat::Tokenizer;


CatGrammar::CatGrammar(TokenizerBase* tokenizer, CatGrammarType grammarType):
	GrammarBase(tokenizer)
{
	Prod rootProduction;
	switch (grammarType)
	{
		default:
		case CatGrammarType::Expression: rootProduction = Prod::Expression; break;
		case CatGrammarType::Full:		 rootProduction = Prod::SourceFile; break;
		case CatGrammarType::Statement:	 rootProduction = Prod::Statement;  break;
	}

	rule(Prod::Root, {prod(rootProduction)}, pass);

	if (grammarType == CatGrammarType::Full || grammarType == CatGrammarType::Statement)
	{
		//A source file has one or more definitions
		rule(Prod::SourceFile, {prod(Prod::Definitions)}, sourceFile);

		//A list of definitions
		rule(Prod::Definitions, {prod(Prod::Definition), prod(Prod::Definitions)}, link);
		rule(Prod::Definitions, {prod(Prod::Definition)}, pass);

		//Possible definitions
		rule(Prod::Definition, {prod(Prod::ClassDefinition)}, pass);
		rule(Prod::Definition, {prod(Prod::FunctionDefinition)}, pass);
		rule(Prod::Definition, {prod(Prod::VariableDefinition), term(one, OneChar::Semicolon)}, pass);
		rule(Prod::Definition, {prod(Prod::InheritanceDefinition), term(one, OneChar::Semicolon)}, pass);

		//Class definition
		rule(Prod::ClassDefinition, {term(id, Identifier::Class), term(id, Identifier::Identifier), term(one, OneChar::BraceOpen), term(one, OneChar::BraceClose)}, classDefinition);
		rule(Prod::ClassDefinition, {term(id, Identifier::Class), term(id, Identifier::Identifier), term(one, OneChar::BraceOpen), prod(Prod::Definitions), term(one, OneChar::BraceClose)}, classDefinition);

		//Inheritance definition
		rule(Prod::InheritanceDefinition, {term(id, Identifier::Inherits), prod(Prod::Identifier)}, inheritanceDefinition);

		//Variable definition (definition)
		rule(Prod::VariableDefinition,  {prod(Prod::Type), term(id, Identifier::Identifier)}, variableDefinition);
		rule(Prod::VariableDefinition, {prod(Prod::Type), term(id, Identifier::Identifier), term(one, OneChar::Assignment), prod(Prod::Expression)}, variableDefinition);

		//Function definition
		rule(Prod::FunctionDefinition, {prod(Prod::Type), term(id, Identifier::Identifier), prod(Prod::FunctionParameters), prod(Prod::ScopeBlock)}, functionDefinition);
		rule(Prod::FunctionParameters, {term(one, OneChar::ParenthesesOpen), term(one, OneChar::ParenthesesClose)}, functionParameterDefinitions);
		rule(Prod::FunctionParameters, {term(one, OneChar::ParenthesesOpen), prod(Prod::FunctionParameterDefinitions), term(one, OneChar::ParenthesesClose)}, functionParameterDefinitions);
		rule(Prod::FunctionParameterDefinitions, {prod(Prod::VariableDeclaration), term(one, OneChar::Comma), prod(Prod::FunctionParameterDefinitions)}, link);
		rule(Prod::FunctionParameterDefinitions, {prod(Prod::VariableDeclaration)}, pass);

		//Scope block
		rule(Prod::ScopeBlock, {term(one, OneChar::BraceOpen), prod(Prod::ScopeBlockStatements), term(one, OneChar::BraceClose)}, scopeBlock);
		rule(Prod::ScopeBlock, {term(one, OneChar::BraceOpen), term(one, OneChar::BraceClose)}, scopeBlock);
		rule(Prod::ScopeBlockStatements, {prod(Prod::Statement), prod(Prod::ScopeBlockStatements)}, link);
		rule(Prod::ScopeBlockStatements, {prod(Prod::Statement)}, pass);

		//All possible statements
		rule(Prod::Statement, {prod(Prod::Return), term(one, OneChar::Semicolon)}, pass);
		rule(Prod::Statement, {prod(Prod::Expression), term(one, OneChar::Semicolon)}, pass);
		rule(Prod::Statement, {prod(Prod::VariableDeclaration), term(one, OneChar::Semicolon)}, pass);
		rule(Prod::Statement, {prod(Prod::IfThen)}, pass);
		rule(Prod::Statement, {prod(Prod::ForLoop)}, pass);

		//Variable declaration (statement)
		rule(Prod::VariableDeclaration, {prod(Prod::Type), term(id, Identifier::Identifier)}, variableDeclaration);
		rule(Prod::VariableDeclaration, {prod(Prod::Type), term(id, Identifier::Identifier), term(one, OneChar::Assignment), prod(Prod::Expression)}, variableDeclaration);

		//If statement
		rule(Prod::IfThen, {term(id, Identifier::If), term(one, OneChar::ParenthesesOpen), prod(Prod::Expression), term(one, OneChar::ParenthesesClose), prod(Prod::ScopeBlock)}, ifStatement);
		rule(Prod::IfThen, {term(id, Identifier::If), term(one, OneChar::ParenthesesOpen), prod(Prod::Expression), term(one, OneChar::ParenthesesClose), prod(Prod::ScopeBlock), prod(Prod::Else)}, ifStatement);
		rule(Prod::Else, {term(id, Identifier::Else), prod(Prod::IfThen)}, pass);
		rule(Prod::Else, {term(id, Identifier::Else), prod(Prod::ScopeBlock)}, pass);

		//Return statement
		rule(Prod::Return, {term(id, Identifier::Return)}, returnStatement);
		rule(Prod::Return, {term(id, Identifier::Return), prod(Prod::Expression)}, returnStatement);

		//For loop statement
		rule(Prod::ForLoop, {term(id, Identifier::For), term(id, Identifier::Identifier), term(id, Identifier::In), prod(Prod::Range), prod(Prod::ScopeBlock)}, forLoop);

		//Range
		rule(Prod::Range, {term(id, Identifier::Range), term(one, OneChar::ParenthesesOpen), prod(Prod::Expression), term(one, OneChar::ParenthesesClose)}, range);
		rule(Prod::Range, {term(id, Identifier::Range), term(one, OneChar::ParenthesesOpen), prod(Prod::Expression), term(one, OneChar::Comma), prod(Prod::Expression), term(one, OneChar::ParenthesesClose)}, range);
		rule(Prod::Range, {term(id, Identifier::Range), term(one, OneChar::ParenthesesOpen), prod(Prod::Expression), term(one, OneChar::Comma), prod(Prod::Expression), term(one, OneChar::Comma), prod(Prod::Expression), term(one, OneChar::ParenthesesClose)}, range);
	}

	//Expressions
	rule(Prod::Expression, {prod(Prod::OperatorP11)}, pass);
	//Operators
	//Precedence from low precedence to high precedence (based on C++ operator precedence)
	//See http://en.cppreference.com/w/cpp/language/operator_precedence

	//= (lowest precedence)
	rule(Prod::OperatorP11, {prod(Prod::OperatorP11), term(one, OneChar::Assignment), prod(Prod::OperatorP10)}, assignmentOperator);
	rule(Prod::OperatorP11, {prod(Prod::OperatorP10)}, pass);

	//|| 
	rule(Prod::OperatorP10, {prod(Prod::OperatorP10), term(two, TwoChar::LogicalOr), prod(Prod::OperatorP9)}, infixOperator);
	rule(Prod::OperatorP10, {prod(Prod::OperatorP9)}, pass);

	//&&
	rule(Prod::OperatorP9, {prod(Prod::OperatorP9), term(two, TwoChar::LogicalAnd), prod(Prod::OperatorP8)}, infixOperator);
	rule(Prod::OperatorP9, {prod(Prod::OperatorP8)}, pass);

	//== != operators
	rule(Prod::OperatorP8, {prod(Prod::OperatorP8), term(two, TwoChar::Equals), prod(Prod::OperatorP7)}, infixOperator);
	rule(Prod::OperatorP8, {prod(Prod::OperatorP8), term(two, TwoChar::NotEquals), prod(Prod::OperatorP7)}, infixOperator);
	rule(Prod::OperatorP8, {prod(Prod::OperatorP7)}, pass);

	//< <= > >= operators
	rule(Prod::OperatorP7, {prod(Prod::OperatorP7), term(one, OneChar::Greater), prod(Prod::OperatorP6)}, infixOperator);
	rule(Prod::OperatorP7, {prod(Prod::OperatorP7), term(one, OneChar::Smaller), prod(Prod::OperatorP6)}, infixOperator);
	rule(Prod::OperatorP7, {prod(Prod::OperatorP7), term(two, TwoChar::GreaterOrEqual), prod(Prod::OperatorP6)}, infixOperator);
	rule(Prod::OperatorP7, {prod(Prod::OperatorP7), term(two, TwoChar::SmallerOrEqual), prod(Prod::OperatorP6)}, infixOperator);
	rule(Prod::OperatorP7, {prod(Prod::OperatorP6)}, pass);

	//infix + - operators
	rule(Prod::OperatorP6, {prod(Prod::OperatorP6), term(one, OneChar::Plus), prod(Prod::OperatorP5)}, infixOperator);
	rule(Prod::OperatorP6, {prod(Prod::OperatorP6), term(one, OneChar::Minus), prod(Prod::OperatorP5)}, infixOperator);
	rule(Prod::OperatorP6, {prod(Prod::OperatorP5)}, pass);

	//* / % operators
	rule(Prod::OperatorP5, {prod(Prod::OperatorP5), term(one, OneChar::Times), prod(Prod::OperatorP4)}, infixOperator);
	rule(Prod::OperatorP5, {prod(Prod::OperatorP5), term(one, OneChar::Divide), prod(Prod::OperatorP4)}, infixOperator);
	rule(Prod::OperatorP5, {prod(Prod::OperatorP5), term(one, OneChar::Modulo), prod(Prod::OperatorP4)}, infixOperator);
	rule(Prod::OperatorP5, {prod(Prod::OperatorP4)}, pass);
	
	// ! and prefix - operators
	rule(Prod::OperatorP4, {term(one, OneChar::Not), prod(Prod::OperatorP3)}, prefixOperator);
	rule(Prod::OperatorP4, {term(one, OneChar::Minus), prod(Prod::OperatorP3)}, prefixOperator);
	rule(Prod::OperatorP4, {prod(Prod::OperatorP3)}, pass);

	// . operator (bla.bla)
	rule(Prod::OperatorP3, {prod(Prod::OperatorP3), term(one, OneChar::Dot), term(id, Identifier::Identifier)}, memberAccessToken);
	rule(Prod::OperatorP3, {prod(Prod::OperatorP3), term(one, OneChar::Dot), term(id, Identifier::Identifier), prod(Prod::FunctionCallArguments)}, memberFunctionCallToken);
	rule(Prod::OperatorP3, {prod(Prod::OperatorP3), term(one, OneChar::BracketOpen), prod(Prod::OperatorP10), term(one, OneChar::BracketClose)}, arrayIndexToken);
	rule(Prod::OperatorP3, {prod(Prod::OperatorP2)}, pass);

	// literals, identifiers, parentheses( ), operator new and function calls (highest precedence)
	rule(Prod::OperatorP2, {prod(Prod::Literal)}, pass);	
	rule(Prod::OperatorP2, {term(one, OneChar::ParenthesesOpen), prod(Prod::OperatorP11), term(one, OneChar::ParenthesesClose)}, pass);
	rule(Prod::OperatorP2, {prod(Prod::Identifier)}, pass);
	rule(Prod::OperatorP2, {prod(Prod::FunctionCall)}, pass);

	if (grammarType == CatGrammarType::Full || grammarType == CatGrammarType::Statement)
	{
		rule(Prod::OperatorP2, {prod(Prod::StaticIdentifier)}, pass);	
		rule(Prod::OperatorP2, { term(id, Identifier::New), prod(Prod::FunctionCall)}, operatorNew);
		rule(Prod::OperatorP2, { term(id, Identifier::New), term(id, Identifier::Array), term(one, OneChar::Smaller), prod(Prod::Type), term(one, OneChar::Greater)}, operatorNewArray);
	}
	
	//Typename
	rule(Prod::Type, {term(one, OneChar::BitwiseAnd), prod(Prod::Identifier) }, typeName);
	rule(Prod::Type, {term(one, OneChar::At), prod(Prod::Identifier) }, typeName);
	rule(Prod::Type, {prod(Prod::Identifier)}, typeName);
	rule(Prod::Type, {term(id, Identifier::Void)}, typeName);
	rule(Prod::Type, {term(id, Identifier::Bool)}, typeName);
	rule(Prod::Type, {term(id, Identifier::Int)}, typeName);
	rule(Prod::Type, {term(id, Identifier::Float)}, typeName);
	rule(Prod::Type, {term(id, Identifier::String)}, typeName);
	//Array type name
	rule(Prod::Type, {term(one, OneChar::BitwiseAnd), term(id, Identifier::Array), term(one, OneChar::Smaller), prod(Prod::Type), term(one, OneChar::Greater)}, arrayTypeName);
	rule(Prod::Type, {term(one, OneChar::At), term(id, Identifier::Array), term(one, OneChar::Smaller), prod(Prod::Type), term(one, OneChar::Greater)}, arrayTypeName);
	rule(Prod::Type, {term(id, Identifier::Array), term(one, OneChar::Smaller), prod(Prod::Type), term(one, OneChar::Greater)}, arrayTypeName);

	rule(Prod::StaticIdentifier, {prod(Prod::Type), term(two, TwoChar::StaticAccessor), term(id, Identifier::Identifier)}, staticIdentifier); 
	rule(Prod::StaticIdentifier, {prod(Prod::StaticIdentifier), term(two, TwoChar::StaticAccessor), term(id, Identifier::Identifier)}, nestedStaticIdentifier); 

	rule(Prod::Identifier, { term(id, Identifier::Identifier) }, identifierToken);

	rule(Prod::FunctionCall, {term(id, Identifier::Identifier), prod(Prod::FunctionCallArguments) }, functionCallToken);
	rule(Prod::FunctionCallArguments, {term(one, OneChar::ParenthesesOpen), term(one, OneChar::ParenthesesClose)}, argumentListToken);
	rule(Prod::FunctionCallArguments, {term(one, OneChar::ParenthesesOpen), prod(Prod::FunctionCallArgumentRepeat), term(one, OneChar::ParenthesesClose)}, argumentListToken);
	rule(Prod::FunctionCallArgumentRepeat, {prod(Prod::Expression), term(one, OneChar::Comma), prod(Prod::FunctionCallArgumentRepeat)}, link);
	rule(Prod::FunctionCallArgumentRepeat, {prod(Prod::Expression)}, pass);

	//Literals
	rule(Prod::Literal, {term(lit, ConstantType::Integer)}, literalToken);
	rule(Prod::Literal, {term(lit, ConstantType::FloatingPoint)}, literalToken);
	rule(Prod::Literal, {term(lit, ConstantType::String)}, literalToken);
	rule(Prod::Literal, {term(lit, ConstantType::Bool)}, literalToken);
	rule(Prod::Literal, {term(id, Identifier::Null) }, literalToken);
	
	setRootProduction(Prod::Root, term(one, OneChar::Eof));

	build();
}


const char* CatGrammar::getProductionName(int production) const
{
	switch((Prod)production)
	{
		default:								return "unknown";
		case Prod::Root:						return "root";
		case Prod::Identifier:					return "Identifier";
		case Prod::StaticIdentifier:			return "StaticIdentifier";
		case Prod::SourceFile:					return "source file";
		case Prod::Definitions:					return "definitions";
		case Prod::Definition:					return "definition";
		case Prod::ClassDefinition:				return "class definition";
		case Prod::ClassContents:				return "class contents";
		case Prod::Declaration:					return "declaration";
		case Prod::InheritanceDefinition:		return "inheritance definition";
		case Prod::FunctionDefinition:			return "function definition";
		case Prod::FunctionParameters:			return "function parameters";
		case Prod::FunctionParameterDefinitions:return "function parameter definitions";
		case Prod::VariableDeclaration:			return "variable declaration";
		case Prod::VariableDefinition:			return "variable definition";
		case Prod::OperatorP2:					return "P2";
		case Prod::OperatorP3:					return "P3";
		case Prod::OperatorP4:					return "P4";
		case Prod::OperatorP5:					return "P5";
		case Prod::OperatorP6:					return "P6";
		case Prod::OperatorP7:					return "P7";
		case Prod::OperatorP8:					return "P8";
		case Prod::OperatorP9:					return "P9";
		case Prod::OperatorP10:					return "P10";
		case Prod::OperatorP11:					return "P11";
		case Prod::Expression:					return "expression";
		case Prod::ExpressionBlock:				return "expression block";
		case Prod::ExpressionBlockContents:		return "expression block contents";
		case Prod::IfThen:						return "if then";
		case Prod::Else:						return "else ";
		case Prod::ElseBody:					return "else body";
		case Prod::ForLoop:						return "for loop";
		case Prod::Range:						return "range";
		case Prod::Continue:					return "continue";
		case Prod::Break:						return "break";
		case Prod::Type:						return "type";
		case Prod::FunctionCall:				return "function call";
		case Prod::FunctionCallArguments:		return "function call arguments";
		case Prod::FunctionCallArgumentRepeat:	return "function call arguments repeat";
		case Prod::Literal:						return "literal";
		case Prod::Assignment:					return "assignment";
		case Prod::ObjectMemberAccess:			return "object member access";
		case Prod::ObjectMemberAccessAction:	return "object member access action";
		case Prod::Return:						return "return";
	}
}


template<typename ItemT>
void unLink(ASTNode* astNode, std::vector<ItemT*>& list)
{
	CatASTNode* catASTNode = static_cast<CatASTNode*>(astNode);
	while (catASTNode != nullptr)
	{
		if (catASTNode->getNodeType() == CatASTNodeType::LinkedList)
		{
			CatLinkNode* linkNode = static_cast<CatLinkNode*>(catASTNode);
			list.push_back(static_cast<ItemT*>(linkNode->releaseMe()));
			catASTNode = linkNode->releaseNext();
			delete linkNode;
		}
		else 
		{
			list.push_back(static_cast<ItemT*>(catASTNode));
			break;
		}
	} 
}


template<typename ItemT>
void unLink(ASTNode* astNode, std::vector<std::unique_ptr<ItemT>>& list)
{
	CatASTNode* catASTNode = static_cast<CatASTNode*>(astNode);
	while (catASTNode != nullptr)
	{
		if (catASTNode->getNodeType() == CatASTNodeType::LinkedList)
		{
			CatLinkNode* linkNode = static_cast<CatLinkNode*>(catASTNode);
			list.emplace_back(static_cast<ItemT*>(linkNode->releaseMe()));
			catASTNode = linkNode->releaseNext();
			delete linkNode;
		}
		else 
		{
			list.emplace_back(static_cast<ItemT*>(catASTNode));
			break;
		}
	} 
}


ASTNode* CatGrammar::pass(const ASTNodeParser& nodeParser)
{
	return nodeParser.getASTNodeByIndex<ASTNode>(0);
}


ASTNode* CatGrammar::link(const ASTNodeParser& nodeParser)
{
	return new CatLinkNode(nodeParser.getASTNodeByIndex<CatASTNode>(0),
						   nodeParser.getASTNodeByIndex<CatASTNode>(1), 
						   nodeParser.getStackLexeme());
}


ASTNode* jitcat::Grammar::CatGrammar::sourceFile(const Parser::ASTNodeParser& nodeParser)
{
	CatASTNode* astNode = nodeParser.getASTNodeByIndex<CatASTNode>(0);
	std::vector<std::unique_ptr<CatDefinition>> definitions;
	Lexeme lexeme = nodeParser.getStackLexeme();
	unLink(nodeParser.getASTNodeByIndex<ASTNode>(0), definitions);
	return new CatSourceFile("none", std::move(definitions), lexeme);
}


ASTNode* jitcat::Grammar::CatGrammar::classDefinition(const Parser::ASTNodeParser& nodeParser)
{
	Lexeme classNameLexeme = nodeParser.getTerminalByIndex(1)->getLexeme();
	std::string className(classNameLexeme);
	CatASTNode* astNode = nodeParser.getASTNodeByIndex<CatASTNode>(0);
	std::vector<std::unique_ptr<CatDefinition>> definitions;
	Lexeme lexeme = nodeParser.getStackLexeme();
	unLink(nodeParser.getASTNodeByIndex<ASTNode>(0), definitions);
	return new CatClassDefinition(className, std::move(definitions), lexeme, classNameLexeme);
}


AST::ASTNode* jitcat::Grammar::CatGrammar::inheritanceDefinition(const Parser::ASTNodeParser& nodeParser)
{
	Lexeme lexeme = nodeParser.getStackLexeme();
	CatIdentifier* identifier = nodeParser.getASTNodeByIndex<CatIdentifier>(0);
	CatTypeNode* typeNode = new CatTypeNode(identifier->getName(), TypeOwnershipSemantics::Value, identifier->getLexeme());
	delete identifier;
	return new CatInheritanceDefinition(typeNode, typeNode->getLexeme(), lexeme);
}


ASTNode* jitcat::Grammar::CatGrammar::functionDefinition(const Parser::ASTNodeParser& nodeParser)
{
	std::string functionName(nodeParser.getTerminalByIndex(0)->getLexeme());
	return new CatFunctionDefinition(nodeParser.getASTNodeByIndex<CatTypeNode>(0), 
									 functionName, nodeParser.getTerminalByIndex(0)->getLexeme(),
									 nodeParser.getASTNodeByIndex<CatFunctionParameterDefinitions>(1),
									 nodeParser.getASTNodeByIndex<CatScopeBlock>(2), nodeParser.getStackLexeme());
}


AST::ASTNode* jitcat::Grammar::CatGrammar::functionParameterDefinitions(const Parser::ASTNodeParser& nodeParser)
{
	std::vector<CatVariableDeclaration*> parameterDefinitions;
	Lexeme lexeme = nodeParser.getStackLexeme();
	unLink(nodeParser.getASTNodeByIndex<ASTNode>(0), parameterDefinitions);
	return new CatFunctionParameterDefinitions(parameterDefinitions, lexeme);
}


AST::ASTNode* jitcat::Grammar::CatGrammar::variableDeclaration(const Parser::ASTNodeParser& nodeParser)
{
	CatTypeNode* type = nodeParser.getASTNodeByIndex<CatTypeNode>(0);
	Lexeme nameLexeme = nodeParser.getTerminalByIndex(0)->getLexeme();
	std::string name(nameLexeme);
	CatTypedExpression* initExpression = nullptr;
	Lexeme assignmentOperatorLexeme = nameLexeme;
	if (nodeParser.getNumItems() > 2)
	{
		//declaration has initialization
		initExpression = nodeParser.getASTNodeByIndex<CatTypedExpression>(1);
		assignmentOperatorLexeme = nodeParser.getTerminalByIndex(1)->getLexeme();
	}
	return new CatVariableDeclaration(type, name, nameLexeme, nodeParser.getStackLexeme(), assignmentOperatorLexeme, initExpression);
}


AST::ASTNode* jitcat::Grammar::CatGrammar::variableDefinition(const Parser::ASTNodeParser & nodeParser)
{
	CatTypeNode* type = nodeParser.getASTNodeByIndex<CatTypeNode>(0);
	std::string name(nodeParser.getTerminalByIndex(0)->getLexeme());
	CatTypedExpression* initExpression = nullptr;
	Lexeme assignmentOperatorLexeme = nodeParser.getTerminalByIndex(0)->getLexeme();
	if (nodeParser.getNumItems() > 2)
	{
		//declaration has initialization
		assignmentOperatorLexeme = nodeParser.getTerminalByIndex(1)->getLexeme();
		initExpression = nodeParser.getASTNodeByIndex<CatTypedExpression>(1);
	}
	return new CatVariableDefinition(type, name, nodeParser.getStackLexeme(), assignmentOperatorLexeme, initExpression);
}


AST::ASTNode* jitcat::Grammar::CatGrammar::arrayTypeName(const Parser::ASTNodeParser& nodeParser)
{
	CatTypeNode* typeNode = nodeParser.getASTNodeByIndex<CatTypeNode>(0);
	TypeOwnershipSemantics ownership = TypeOwnershipSemantics::Owned;
	const ParseToken* firstTerminal = nodeParser.getTerminalByIndex(0);
	if (firstTerminal->getTokenID() == OneCharToken::getID())
	{
		if ((OneChar)firstTerminal->getTokenSubType() == OneChar::BitwiseAnd)
		{
			ownership = TypeOwnershipSemantics::Weak;
		}
		else  if ((OneChar)firstTerminal->getTokenSubType() == OneChar::At)
		{
			ownership = TypeOwnershipSemantics::Value;
		}
	}
	return new CatTypeNode(typeNode, ownership, nodeParser.getStackLexeme());
}


ASTNode* jitcat::Grammar::CatGrammar::typeName(const Parser::ASTNodeParser& nodeParser)
{
	CatIdentifier* identifierNode = nodeParser.getASTNodeByIndex<CatIdentifier>(0);
	Identifier identifierType;
	std::string identifierName;
	Lexeme identifierLexeme;
	TypeOwnershipSemantics ownership = TypeOwnershipSemantics::Value;
	if (identifierNode != nullptr)
	{
		identifierType = Identifier::Identifier;
		identifierName = identifierNode->name;
		identifierLexeme = identifierNode->getLexeme();
		const OneCharToken* token = static_cast<const OneCharToken*>(nodeParser.getTerminalByIndex(0));
		if (token != nullptr)
		{
			switch ((OneChar)token->getTokenSubType())
			{
				default:
				case OneChar::BitwiseAnd:		ownership = TypeOwnershipSemantics::Weak;	break;
				//case OneChar::BitwiseAnd:	ownership = TypeOwnershipSemantics::Shared; break;
				case OneChar::At:			ownership = TypeOwnershipSemantics::Value;	break;
			}
		}
		else
		{
			ownership = TypeOwnershipSemantics::Owned;
		}
		delete identifierNode;
	}
	else
	{
		identifierType = static_cast<Identifier>(nodeParser.getTerminalByIndex(0)->getTokenSubType());
		identifierLexeme = nodeParser.getStackLexeme();
	}
	
	CatGenericType type;
	switch (identifierType)
	{
		case Identifier::Bool:		 type = CatGenericType::boolType; break;
		case Identifier::Int:		 type = CatGenericType::intType; break;
		case Identifier::Float:		 type = CatGenericType::floatType; break;
		case Identifier::String:	 type = CatGenericType::stringType; break;
		case Identifier::Void:		 type = CatGenericType::voidType;	break;
		case Identifier::Identifier:
		{
			return new CatTypeNode(identifierName, ownership, identifierLexeme);
		}
	}
	return new CatTypeNode(type, identifierLexeme);
}


AST::ASTNode* jitcat::Grammar::CatGrammar::ifStatement(const Parser::ASTNodeParser& nodeParser)
{
	CatTypedExpression* condition = nodeParser.getASTNodeByIndex<CatTypedExpression>(0);
	CatScopeBlock* ifBody = nodeParser.getASTNodeByIndex<CatScopeBlock>(1);
	CatStatement* elseNode = nodeParser.getASTNodeByIndex<CatStatement>(2);
	return new CatIfStatement(condition, ifBody, nodeParser.getStackLexeme(), elseNode);
}


AST::ASTNode* jitcat::Grammar::CatGrammar::returnStatement(const Parser::ASTNodeParser& nodeParser)
{
	CatTypedExpression* returnExpression = nullptr;
	if (nodeParser.getNumItems() > 0)
	{
		returnExpression = nodeParser.getASTNodeByIndex<CatTypedExpression>(0);
	}
	return new CatReturnStatement(nodeParser.getStackLexeme(), returnExpression);
}


AST::ASTNode* jitcat::Grammar::CatGrammar::scopeBlock(const Parser::ASTNodeParser& nodeParser)
{
	std::vector<CatStatement*> statements;
	Lexeme lexeme = nodeParser.getStackLexeme();
	unLink(nodeParser.getASTNodeByIndex<ASTNode>(0), statements);
	return new CatScopeBlock(statements, lexeme);
}


//For loop statement
//rule(Prod::ForLoop, {term(id, Identifier::For), term(id, Identifier::Identifier), term(id, Identifier::In), prod(Prod::Range), prod(Prod::ScopeBlock)}, forLoop);
AST::ASTNode* jitcat::Grammar::CatGrammar::forLoop(const Parser::ASTNodeParser& nodeParser)
{
	const ParseToken* token = nodeParser.getTerminalByIndex(1);
	CatRange* rangeNode = nodeParser.getASTNodeByIndex<CatRange>(0);
	CatScopeBlock* loopBody = nodeParser.getASTNodeByIndex<CatScopeBlock>(1);

	return new CatForLoop(nodeParser.getStackLexeme(), token->getLexeme(), rangeNode, loopBody);
}


AST::ASTNode* jitcat::Grammar::CatGrammar::range(const Parser::ASTNodeParser& nodeParser)
{
	CatTypedExpression* rangeMin = nodeParser.getASTNodeByIndex<CatTypedExpression>(0);
	CatTypedExpression* rangeMax = nodeParser.getASTNodeByIndex<CatTypedExpression>(1);
	CatTypedExpression* rangeStep = nodeParser.getASTNodeByIndex<CatTypedExpression>(2);
	if (rangeMax == nullptr)
	{
		rangeMax = rangeMin;
		rangeMin = nullptr;
		return new CatRange(rangeMax, nodeParser.getStackLexeme());
	}
	else if (rangeStep == nullptr)
	{
		return new CatRange(rangeMin, rangeMax, nodeParser.getStackLexeme());
	}
	else
	{
		return new CatRange(rangeMin, rangeMax, rangeStep, nodeParser.getStackLexeme());
	}
}


ASTNode* CatGrammar::assignmentOperator(const ASTNodeParser & nodeParser)
{
	return new CatAssignmentOperator(nodeParser.getASTNodeByIndex<CatTypedExpression>(0),
									 nodeParser.getASTNodeByIndex<CatTypedExpression>(1), 
									 nodeParser.getStackLexeme(), nodeParser.getTerminalByIndex(0)->getLexeme());

}


ASTNode* CatGrammar::infixOperator(const ASTNodeParser& nodeParser)
{
	CatTypedExpression* lhs = nodeParser.getASTNodeByIndex<CatTypedExpression>(0);
	CatTypedExpression* rhs = nodeParser.getASTNodeByIndex<CatTypedExpression>(1);
	
	const ParseToken* infix = nullptr;
	if (nodeParser.getNumItems() <= 3)
	{
		//This is a parse of the form "exp + exp"
		infix = nodeParser.getTerminalByIndex(0);
	}
	else
	{
		//This is a parse of the form "(exp) + exp"
		infix = nodeParser.getTerminalByIndex(2);
	}

	CatInfixOperatorType operatorType = CatInfixOperatorType::Plus;
	if (infix->getTokenID() == Cat::one)
	{
		switch ((OneChar)infix->getTokenSubType())
		{
			default:
			case OneChar::Plus:			operatorType = CatInfixOperatorType::Plus;		break;
			case OneChar::Minus:		operatorType = CatInfixOperatorType::Minus;		break;			
			case OneChar::Times:		operatorType = CatInfixOperatorType::Multiply;	break;			
			case OneChar::Divide:		operatorType = CatInfixOperatorType::Divide;		break;			
			case OneChar::Modulo:		operatorType = CatInfixOperatorType::Modulo;		break;
			case OneChar::Greater:		operatorType = CatInfixOperatorType::Greater;		break;
			case OneChar::Smaller:		operatorType = CatInfixOperatorType::Smaller;		break;
		}
	}
	else if (infix->getTokenID() == Cat::two)
	{
		switch ((TwoChar)infix->getTokenSubType())
		{
			case TwoChar::GreaterOrEqual:	operatorType = CatInfixOperatorType::GreaterOrEqual;	break;
			case TwoChar::SmallerOrEqual:	operatorType = CatInfixOperatorType::SmallerOrEqual;	break;
			case TwoChar::Equals:			operatorType = CatInfixOperatorType::Equals;			break;
			case TwoChar::NotEquals:		operatorType = CatInfixOperatorType::NotEquals;		break;
			case TwoChar::LogicalAnd:		operatorType = CatInfixOperatorType::LogicalAnd;		break;
			case TwoChar::LogicalOr:		operatorType = CatInfixOperatorType::LogicalOr;		break;
		}
	}

	return new CatInfixOperator(lhs, rhs, operatorType, nodeParser.getStackLexeme(), nodeParser.getTerminalByIndex(0)->getLexeme());
}


ASTNode* CatGrammar::prefixOperator(const ASTNodeParser& nodeParser)
{
	const ParseToken* prefix = nodeParser.getTerminalByIndex(0);
	CatPrefixOperator::Operator op = CatPrefixOperator::Operator::Not;
	if (prefix->getTokenID() == Cat::one)
	{
		switch ((OneChar)prefix->getTokenSubType())
		{
		default:
		case OneChar::Not:
			op = CatPrefixOperator::Operator::Not;
			break;
		case OneChar::Minus:
			op = CatPrefixOperator::Operator::Minus;
			break;
		}
	}
	return new CatPrefixOperator(nodeParser.getStackLexeme(), op, nodeParser.getASTNodeByIndex<CatTypedExpression>(0));
}


AST::ASTNode* jitcat::Grammar::CatGrammar::operatorNew(const Parser::ASTNodeParser& nodeParser)
{
	CatMemberFunctionCall* callNode = nodeParser.getASTNodeByIndex<CatMemberFunctionCall>(0);
	
	return new CatOperatorNew(callNode, callNode->getFunctionName(), nodeParser.getStackLexeme());
}


AST::ASTNode* jitcat::Grammar::CatGrammar::operatorNewArray(const Parser::ASTNodeParser& nodeParser)
{
	CatTypeNode* arrayItemTypeNode = nodeParser.getASTNodeByIndex<CatTypeNode>(0);
	return new CatOperatorNewArray(arrayItemTypeNode, nodeParser.getStackLexeme());
}


ASTNode* CatGrammar::literalToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* literalToken = nodeParser.getTerminalByIndex(0);
	if (literalToken->getTokenID() == Cat::lit)
	{
		switch ((ConstantType)literalToken->getTokenSubType())
		{
			default:	return nullptr;
			case ConstantType::Integer:
			{
				CatLiteral* intLiteral = new CatLiteral(atoi(literalToken->getLexeme().data()), nodeParser.getStackLexeme());
				return intLiteral;
			}
			case ConstantType::FloatingPoint:
			{
				CatLiteral* floatLiteral = new CatLiteral((float)atof(literalToken->getLexeme().data()), nodeParser.getStackLexeme());
				return floatLiteral;
			}
			case ConstantType::String:
			{
				CatLiteral* stringLiteral = new CatLiteral(std::string(literalToken->getLexeme().data() + 1, literalToken->getLexeme().length() - 2), nodeParser.getStackLexeme());
				return stringLiteral;
			}
			case ConstantType::Bool:
			{
				//if the first character of the match is 't' then it's "true"
				CatLiteral* boolLiteral = new CatLiteral(literalToken->getLexeme().data()[0] == 't'
														 || literalToken->getLexeme().data()[0] == 'T', nodeParser.getStackLexeme());
				
				return boolLiteral;
			}
			case ConstantType::Char:
			{
				CatLiteral* charLiteral = new CatLiteral(literalToken->getLexeme().data()[0], nodeParser.getStackLexeme());
				return charLiteral;
			}			
		}
	}
	else if (literalToken->getTokenID() == Cat::id && (Identifier)literalToken->getTokenSubType() == Identifier::Null)
	{
		return new CatLiteral((Reflectable*)nullptr, CatGenericType::nullptrType, nodeParser.getStackLexeme());
	}
	return nullptr;
}


ASTNode* CatGrammar::identifierToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* token = nodeParser.getTerminalByIndex(0);
	std::string variableName(token->getLexeme());
	return new CatIdentifier(variableName, nodeParser.getStackLexeme());
}


AST::ASTNode* jitcat::Grammar::CatGrammar::staticIdentifier(const Parser::ASTNodeParser& nodeParser)
{
	CatTypeNode* scopeTypeNode = nodeParser.getASTNodeByIndex<CatTypeNode>(0);
	const ParseToken* token = nodeParser.getTerminalByIndex(1);
	std::string variableName(token->getLexeme());
	return new CatStaticIdentifier(scopeTypeNode, variableName, nodeParser.getStackLexeme());
}


AST::ASTNode* jitcat::Grammar::CatGrammar::nestedStaticIdentifier(const Parser::ASTNodeParser& nodeParser)
{
	CatStaticIdentifier* staticIdentifier = nodeParser.getASTNodeByIndex<CatStaticIdentifier>(0);
	const ParseToken* token = nodeParser.getTerminalByIndex(1);
	std::string variableName(token->getLexeme());
	return new CatStaticIdentifier(staticIdentifier, variableName, nodeParser.getStackLexeme());
}


ASTNode* CatGrammar::argumentListToken(const ASTNodeParser& nodeParser)
{
	Lexeme lexeme = nodeParser.getStackLexeme();
	std::vector<CatTypedExpression*> argumentList;
	unLink(nodeParser.getASTNodeByIndex<ASTNode>(0), argumentList);
	return new CatArgumentList(lexeme, argumentList);
}


ASTNode* CatGrammar::functionCallToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* nameToken = nodeParser.getTerminalByIndex(0);
	std::string functionName(nameToken->getLexeme());
	CatArgumentList* arguments = nodeParser.getASTNodeByIndex<CatArgumentList>(0);
	if (CatFunctionCall::isBuiltInFunction(functionName.c_str(), (int)arguments->getNumArguments()))
	{
		return new CatFunctionCall(functionName, nameToken->getLexeme(), arguments, nodeParser.getStackLexeme());
	}
	else
	{
		std::string lowerName = Tools::toLowerCase(functionName);
		return new CatMemberFunctionCall(lowerName, nameToken->getLexeme(), nullptr, arguments, nodeParser.getStackLexeme());
	}
}


ASTNode* CatGrammar::memberAccessToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* nameToken = nodeParser.getTerminalByIndex(1);
	std::string memberName(nameToken->getLexeme());
	CatTypedExpression* base = nodeParser.getASTNodeByIndex<CatTypedExpression>(0);
	return new CatMemberAccess(base, memberName, nodeParser.getStackLexeme());
}


ASTNode* CatGrammar::memberFunctionCallToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* nameToken = nodeParser.getTerminalByIndex(1);
	std::string functionName(nameToken->getLexeme());
	CatTypedExpression* base = nodeParser.getASTNodeByIndex<CatTypedExpression>(0);
	CatArgumentList* arguments = nodeParser.getASTNodeByIndex<CatArgumentList>(1);
	return new CatMemberFunctionCall(functionName, nameToken->getLexeme(), base, arguments, nodeParser.getStackLexeme());
}


ASTNode* CatGrammar::arrayIndexToken(const ASTNodeParser& nodeParser)
{
	CatTypedExpression* base = nodeParser.getASTNodeByIndex<CatTypedExpression>(0);
	CatTypedExpression* index = nodeParser.getASTNodeByIndex<CatTypedExpression>(1);
	return new CatArrayIndex(base, index, nodeParser.getStackLexeme());
}
