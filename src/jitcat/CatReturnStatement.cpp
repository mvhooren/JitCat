/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatReturnStatement.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypedExpression.h"

using namespace jitcat;
using namespace jitcat::AST;


CatReturnStatement::CatReturnStatement(const Tokenizer::Lexeme& lexeme, CatTypedExpression* returnExpression):
	CatTypedExpression(lexeme),
	returnExpression(returnExpression)
{
}


CatReturnStatement::~CatReturnStatement()
{
}


void CatReturnStatement::print() const
{
	Tools::CatLog::log("return");
	if (returnExpression.get() != nullptr)
	{
		Tools::CatLog::log(" ");
		returnExpression->print();
	}
}


CatASTNodeType CatReturnStatement::getNodeType()
{
	return CatASTNodeType::ReturnStatement;
}


std::any CatReturnStatement::execute(CatRuntimeContext* runtimeContext)
{
	return std::any();
}


bool CatReturnStatement::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	return false;
}


CatGenericType CatReturnStatement::getType() const
{
	return CatGenericType();
}


bool CatReturnStatement::isConst() const
{
	return false;
}


CatTypedExpression* CatReturnStatement::constCollapse(CatRuntimeContext* compileTimeContext)
{
	return this;
}
