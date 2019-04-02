/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatReturnStatement.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/ExpressionErrorManager.h"

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
	if (returnExpression != nullptr)
	{
		return returnExpression->execute(runtimeContext);
	}
	else
	{
		return std::any();
	}
}


bool CatReturnStatement::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	CatFunctionDefinition* currentFunction = compiletimeContext->getCurrentFunction();
	if (returnExpression == nullptr && !currentFunction->getReturnTypeNode()->getType().isVoidType())
	{
		errorManager->compiledWithError("Void returned where a non-void type was expected.", errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	else if (returnExpression != nullptr)
	{
		if (!returnExpression->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			return false;
		}
		else if (returnExpression->getType() != currentFunction->getReturnTypeNode()->getType())
		{
			errorManager->compiledWithError("Returned type does not match function return type.", errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	return true;
}


CatGenericType CatReturnStatement::getType() const
{
	if (returnExpression != nullptr)
	{
		return returnExpression->getType();
	}
	return CatGenericType::voidType;
}


bool CatReturnStatement::isConst() const
{
	if (returnExpression != nullptr)
	{
		return returnExpression->isConst();
	}
	else
	{
		return true;
	}
}


CatTypedExpression* CatReturnStatement::constCollapse(CatRuntimeContext* compileTimeContext)
{
	if (returnExpression != nullptr)
	{
		ASTHelper::updatePointerIfChanged(returnExpression, returnExpression->constCollapse(compileTimeContext));
	}
	return this;
}
