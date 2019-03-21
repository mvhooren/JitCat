/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatIfStatement.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatTypedExpression.h"

using namespace jitcat;
using namespace jitcat::AST;


CatIfStatement::CatIfStatement(CatTypedExpression* condition, CatScopeBlock* ifBody, const Tokenizer::Lexeme& lexeme, CatASTNode* elseNode):
	CatStatement(lexeme),
	condition(condition),
	ifBody(ifBody),
	elseNode(elseNode)
{

}


void CatIfStatement::print() const
{
	Tools::CatLog::log("if (");
	condition->print();
	Tools::CatLog::log(")\n");
	ifBody->print();
	if (elseNode != nullptr)
	{
		Tools::CatLog::log("else");
		if (elseNode->getNodeType() == CatASTNodeType::ScopeBlock)
		{
			Tools::CatLog::log("\n");
		}
		elseNode->print();
	}
}


CatASTNodeType CatIfStatement::getNodeType()
{
	return CatASTNodeType::IfStatement;
}


bool jitcat::AST::CatIfStatement::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	return false;
}
