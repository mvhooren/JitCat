/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatErrorExpression.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


CatErrorExpression::CatErrorExpression(const std::string& contents, const std::string& errorMessage, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	contents(contents),
	errorMessage(errorMessage)
{
}


CatErrorExpression::CatErrorExpression(const CatErrorExpression& other):
	CatTypedExpression(other),
	contents(other.contents),
	errorMessage(other.errorMessage)
{
}


CatASTNode* CatErrorExpression::copy() const
{
	return new CatErrorExpression(*this);
}


const CatGenericType& CatErrorExpression::getType() const
{
	return CatGenericType::unknownType;
}


void CatErrorExpression::print() const
{
	CatLog::log(contents);
}


bool CatErrorExpression::isConst() const
{
	return false;
}


CatTypedExpression* CatErrorExpression::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	return this;
}


CatASTNodeType CatErrorExpression::getNodeType() const
{
	return CatASTNodeType::ErrorExpression;
}


std::any CatErrorExpression::execute(CatRuntimeContext* runtimeContext)
{
	return std::any();
}


bool CatErrorExpression::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	errorManager->compiledWithError(errorMessage, errorContext, compiletimeContext->getContextName(), lexeme);
	return false;
}
