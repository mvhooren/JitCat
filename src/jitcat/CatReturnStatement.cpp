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


jitcat::AST::CatReturnStatement::CatReturnStatement(const CatReturnStatement& other):
	CatTypedExpression(other),
	returnExpression(static_cast<CatTypedExpression*>(other.returnExpression->copy()))
{
}


CatReturnStatement::~CatReturnStatement()
{
}


CatASTNode* jitcat::AST::CatReturnStatement::copy() const
{
	return new CatReturnStatement(*this);
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


CatASTNodeType CatReturnStatement::getNodeType() const
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
		else if (!returnExpression->getType().compare(currentFunction->getReturnTypeNode()->getType(), false, false))
		{
			errorManager->compiledWithError("Returned type does not match function return type.", errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		IndirectionConversionMode conversionMode = IndirectionConversionMode::None;
		bool indirectionConversionSuccess = ASTHelper::doIndirectionConversion(returnExpression, currentFunction->getReturnTypeNode()->getType(), false, conversionMode);
		//To silence unused variable warning in release builds.
		(void)indirectionConversionSuccess;
		assert(indirectionConversionSuccess);

	}
	return true;
}


const CatGenericType& CatReturnStatement::getType() const
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


CatStatement* CatReturnStatement::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (returnExpression != nullptr)
	{
		ASTHelper::updatePointerIfChanged(returnExpression, returnExpression->constCollapse(compileTimeContext, errorManager, errorContext));
	}
	return this;
}


std::optional<bool> jitcat::AST::CatReturnStatement::checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected)
{
	allControlPathsReturn = true;
	return allControlPathsReturn;
}


const CatTypedExpression* jitcat::AST::CatReturnStatement::getReturnExpression() const
{
	return returnExpression.get();
}
