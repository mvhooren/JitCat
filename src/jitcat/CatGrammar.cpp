/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatGrammar.h"
#include "ASTNodeParser.h"
#include "CatASTNodes.h"
#include "CatScopeRoot.h"
#include "CatTokenIds.h"
#include "ConstantToken.h"
#include "Lexeme.h"
#include "OneCharToken.h"
#include "Tools.h"

#include <stdlib.h>

using namespace Cat;

CatGrammar::CatGrammar(Tokenizer* tokenizer):
	Grammar(tokenizer)
{
	rule(Prod::Root, {prod(Prod::Expression)}, pass);
	
	//Expressions
	rule(Prod::Expression, {prod(Prod::OperatorP10)}, pass);
	//Operators
	//Precedence from low precedence to high precedence (based on C++ operator precedence)
	//See http://en.cppreference.com/w/cpp/language/operator_precedence

	//= (lowest precedence)
	//Assignment cannot work while the type checking process actually executes expressions. It should be possible to fix this now.
	/*rule(Prod::OperatorP11, prod(Prod::OperatorP11), term(one, OneChar::Assignment), prod(Prod::OperatorP10), infixOperator);
	rule(Prod::OperatorP11, prod(Prod::OperatorP10), pass);*/

	//|| (lowest precedence)
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

	// literals, identifiers and ( ) (highest precedence)
	rule(Prod::OperatorP2, {prod(Prod::Literal)}, pass);	
	rule(Prod::OperatorP2, {term(one, OneChar::ParenthesesOpen), prod(Prod::OperatorP10), term(one, OneChar::ParenthesesClose)}, pass);
	rule(Prod::OperatorP2, {term(id, Identifier::Identifier)}, identifierToken);
	rule(Prod::OperatorP2, {term(id, Identifier::Identifier), prod(Prod::FunctionCallArguments)}, functionCallToken);

	rule(Prod::FunctionCallArguments, {term(one, OneChar::ParenthesesOpen), term(one, OneChar::ParenthesesClose)}, argumentListToken);
	rule(Prod::FunctionCallArguments, {term(one, OneChar::ParenthesesOpen), prod(Prod::FunctionCallArgumentRepeat), term(one, OneChar::ParenthesesClose)}, argumentListToken);
	rule(Prod::FunctionCallArgumentRepeat, {prod(Prod::Expression), term(one, OneChar::Comma), prod(Prod::FunctionCallArgumentRepeat)}, link);
	rule(Prod::FunctionCallArgumentRepeat, {prod(Prod::Expression)}, pass);

	//Literals
	rule(Prod::Literal, {term(lit, ConstantType::Integer)}, literalToken);
	rule(Prod::Literal, {term(lit, ConstantType::FloatingPoint)}, literalToken);
	rule(Prod::Literal, {term(lit, ConstantType::String)}, literalToken);
	rule(Prod::Literal, {term(lit, ConstantType::Bool)}, literalToken);
	
	setRootProduction(Prod::Root, term(one, OneChar::Eof));

	build();
}


const char* CatGrammar::getProductionName(int production) const
{
	switch((Prod)production)
	{
		default:								return "unknown";
		case Prod::Root:						return "root";
		case Prod::ClassDefinition:				return "class definition";
		case Prod::ClassContents:				return "class contents";
		case Prod::Declaration:					return "declaration";
		case Prod::FunctionDefinition:			return "function definition";
		case Prod::FunctionParameterDefinition:	return "function parameter definition";
		case Prod::FunctionParameterDefRepeat:	return "function parameter definition repeat";
		case Prod::VariableDeclaration:			return "variable declaration";
		case Prod::VariableDeclarationTail:		return "variable declaration tail";
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
		case Prod::FlowControl:					return "flow control";
		case Prod::IfThen:						return "if then";
		case Prod::Else:						return "else ";
		case Prod::ElseBody:					return "else body";
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


bool CatGrammar::isTypedExpression(CatASTNodeType node)
{
	return	node == CatASTNodeType::Literal
			|| node == CatASTNodeType::Identifier
			|| node == CatASTNodeType::InfixOperator
			|| node == CatASTNodeType::PrefixOperator
			|| node == CatASTNodeType::FunctionCall
			|| node == CatASTNodeType::MemberAccess
			|| node == CatASTNodeType::ArrayIndex
			|| node == CatASTNodeType::MemberFunctionCall;
}


ASTNode* CatGrammar::pass(const ASTNodeParser& nodeParser)
{
	return nodeParser.getASTNodeByIndex(0);
}


ASTNode* CatGrammar::link(const ASTNodeParser& nodeParser)
{
	return new CatLinkNode(static_cast<CatASTNode*>(nodeParser.getASTNodeByIndex(0)),
						   static_cast<CatASTNode*>(nodeParser.getASTNodeByIndex(1)));
}


ASTNode* CatGrammar::infixOperator(const ASTNodeParser& nodeParser)
{
	CatInfixOperator* oper = new CatInfixOperator();
	oper->lhs.reset(static_cast<CatTypedExpression*>(nodeParser.getASTNodeByIndex(0)));
	oper->rhs.reset(static_cast<CatTypedExpression*>(nodeParser.getASTNodeByIndex(1)));
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
	
	oper->oper = CatInfixOperatorType::Plus;
	if (infix->getTokenID() == Cat::one)
	{
		switch ((OneChar)infix->getTokenSubType())
		{
			default:
			case OneChar::Assignment:	oper->oper = CatInfixOperatorType::Assign;		break;
			case OneChar::Plus:			oper->oper = CatInfixOperatorType::Plus;		break;
			case OneChar::Minus:		oper->oper = CatInfixOperatorType::Minus;		break;			
			case OneChar::Times:		oper->oper = CatInfixOperatorType::Multiply;	break;			
			case OneChar::Divide:		oper->oper = CatInfixOperatorType::Divide;		break;			
			case OneChar::Modulo:		oper->oper = CatInfixOperatorType::Modulo;		break;
			case OneChar::Greater:		oper->oper = CatInfixOperatorType::Greater;		break;
			case OneChar::Smaller:		oper->oper = CatInfixOperatorType::Smaller;		break;
		}
	}
	else if (infix->getTokenID() == Cat::two)
	{
		switch ((TwoChar)infix->getTokenSubType())
		{
			case TwoChar::GreaterOrEqual:		oper->oper = CatInfixOperatorType::GreaterOrEqual;	break;
			case TwoChar::SmallerOrEqual:		oper->oper = CatInfixOperatorType::SmallerOrEqual;	break;
			case TwoChar::Equals:				oper->oper = CatInfixOperatorType::Equals;			break;
			case TwoChar::NotEquals:			oper->oper = CatInfixOperatorType::NotEquals;		break;
			case TwoChar::LogicalAnd:			oper->oper = CatInfixOperatorType::LogicalAnd;		break;
			case TwoChar::LogicalOr:			oper->oper = CatInfixOperatorType::LogicalOr;		break;
		}
	}

	return oper;
}


ASTNode* CatGrammar::prefixOperator(const ASTNodeParser& nodeParser)
{
	CatPrefixOperator* oper = new CatPrefixOperator();
	oper->rhs.reset(static_cast<CatTypedExpression*>(nodeParser.getASTNodeByIndex(0)));
	const ParseToken* prefix = nodeParser.getTerminalByIndex(0);
	oper->oper = CatPrefixOperator::Operator::Not;
	if (prefix->getTokenID() == Cat::one)
	{
		switch ((OneChar)prefix->getTokenSubType())
		{
			default:
			case OneChar::Not:
				oper->oper = CatPrefixOperator::Operator::Not;
				break;
			case OneChar::Minus:
				oper->oper = CatPrefixOperator::Operator::Minus;
				break;
		}
	}
	return oper;
}


ASTNode* CatGrammar::literalToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* literalToken = nodeParser.getTerminalByIndex(0);
	if (literalToken->getTokenID() == Cat::lit)
	{
		switch ((ConstantType)literalToken->getTokenSubType())
		{
			default:
				return new CatLiteral();
			case ConstantType::Integer:
			{
				CatLiteral* intLiteral = new CatLiteral(atoi(literalToken->getLexeme()->getDataPointer()));
				return intLiteral;
			}
			case ConstantType::FloatingPoint:
			{
				CatLiteral* floatLiteral = new CatLiteral((float)atof(literalToken->getLexeme()->getDataPointer()));
				return floatLiteral;
			}
			case ConstantType::String:
			{
				CatLiteral* stringLiteral = new CatLiteral(std::string(literalToken->getLexeme()->getDataPointer() + 1, literalToken->getLexeme()->length - 2));
				return stringLiteral;
			}
			case ConstantType::Bool:
			{
				//if the first character of the match is 't' then it's "true"
				CatLiteral* boolLiteral = new CatLiteral(literalToken->getLexeme()->getDataPointer()[0] == 't'
														 || literalToken->getLexeme()->getDataPointer()[0] == 'T');
				
				return boolLiteral;
			}
			case ConstantType::Char:
			{
				CatLiteral* charLiteral = new CatLiteral(literalToken->getLexeme()->getDataPointer()[0]);
				return charLiteral;
			}			
		}
	}
	return new CatLiteral();
}


ASTNode* CatGrammar::identifierToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* token = nodeParser.getTerminalByIndex(0);
	std::string variableName(token->getLexeme()->getDataPointer(), token->getLexeme()->length);
	return new CatIdentifier(variableName, static_cast<CatRuntimeContext*>(nodeParser.getContext()));
}


ASTNode* CatGrammar::argumentListToken(const ASTNodeParser& nodeParser)
{
	CatArgumentList* argumentList = new CatArgumentList();
	CatASTNode* astNode = static_cast<CatASTNode*>(nodeParser.getASTNodeByIndex(0));
	while (astNode != nullptr)
	{
		if (isTypedExpression(astNode->getNodeType()))
		{
			argumentList->arguments.emplace_back(static_cast<CatTypedExpression*>(astNode));
			break;
		}
		else if (astNode->getNodeType() == CatASTNodeType::LinkedList)
		{
			CatLinkNode* linkNode = static_cast<CatLinkNode*>(astNode);
			argumentList->arguments.emplace_back(static_cast<CatTypedExpression*>(linkNode->me.get()));
			astNode = linkNode->next.get();
			linkNode->me.release();
			linkNode->next.release();
			delete linkNode;
		}
		else
		{
			//Error. Should not get here.
			break;
		}
	} 
	return argumentList;
}


ASTNode* CatGrammar::functionCallToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* nameToken = nodeParser.getTerminalByIndex(0);
	std::string functionName(nameToken->getLexeme()->getDataPointer(), nameToken->getLexeme()->length);
	CatArgumentList* arguments = static_cast<CatArgumentList*>(nodeParser.getASTNodeByIndex(0));
	if (CatFunctionCall::isBuiltInFunction(functionName.c_str(), (int)arguments->arguments.size()))
	{
		return new CatFunctionCall(functionName, arguments);
	}
	else
	{
		CatRuntimeContext* runtimeContext = static_cast<CatRuntimeContext*>(nodeParser.getContext());
		if (runtimeContext != nullptr)
		{
			RootTypeSource source = RootTypeSource::None;
			std::string lowerName = Tools::toLowerCase(functionName);
			MemberFunctionInfo* functionInfo = runtimeContext->findFunction(lowerName, source);
			if (functionInfo != nullptr)
			{
				return new CatMemberFunctionCall(lowerName, new CatScopeRoot(source, runtimeContext), arguments);
			}
		}
		return new CatFunctionCall(functionName, arguments);
	}
}


ASTNode* CatGrammar::memberAccessToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* nameToken = nodeParser.getTerminalByIndex(1);
	std::string memberName(nameToken->getLexeme()->getDataPointer(), nameToken->getLexeme()->length);
	CatTypedExpression* base = static_cast<CatTypedExpression*>(nodeParser.getASTNodeByIndex(0));
	return new CatMemberAccess(base, memberName);
}


ASTNode* CatGrammar::memberFunctionCallToken(const ASTNodeParser& nodeParser)
{
	const ParseToken* nameToken = nodeParser.getTerminalByIndex(1);
	std::string functionName(nameToken->getLexeme()->getDataPointer(), nameToken->getLexeme()->length);
	CatTypedExpression* base = static_cast<CatTypedExpression*>(nodeParser.getASTNodeByIndex(0));
	CatArgumentList* arguments = static_cast<CatArgumentList*>(nodeParser.getASTNodeByIndex(1));
	return new CatMemberFunctionCall(functionName, base, arguments);
}


ASTNode* CatGrammar::arrayIndexToken(const ASTNodeParser& nodeParser)
{
	CatTypedExpression* base = static_cast<CatTypedExpression*>(nodeParser.getASTNodeByIndex(0));
	CatTypedExpression* index = static_cast<CatTypedExpression*>(nodeParser.getASTNodeByIndex(1));
	return new CatArrayIndex(base, index);
}
