/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatInfixOperator.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatStaticFunctionCall.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/InfixOperatorOptimizer.h"
#include "jitcat/ASTHelper.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


CatInfixOperator::CatInfixOperator(CatTypedExpression* lhs, CatTypedExpression* rhs, CatInfixOperatorType operatorType, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& operatorLexeme):
	CatTypedExpression(lexeme),
	resultType(CatGenericType::unknownType),
	rhs(rhs),
	lhs(lhs),
	oper(operatorType),
	operatorLexeme(operatorLexeme)
{
}


jitcat::AST::CatInfixOperator::CatInfixOperator(const CatInfixOperator& other):
	CatTypedExpression(other),
	resultType(CatGenericType::unknownType),
	rhs(static_cast<CatTypedExpression*>(other.rhs->copy())),
	lhs(static_cast<CatTypedExpression*>(other.lhs->copy())),
	oper(other.oper),
	operatorLexeme(other.operatorLexeme)
{
}


CatASTNode* jitcat::AST::CatInfixOperator::copy() const
{
	return new CatInfixOperator(*this);
}


const CatGenericType& CatInfixOperator::getType() const
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator->getType();
	}
	else
	{
		return resultType;
	}
}


bool CatInfixOperator::isConst() const 
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator->isConst();
	}
	else
	{
		return lhs->isConst() && rhs->isConst();
	}
}


CatTypedExpression* CatInfixOperator::constCollapse(CatRuntimeContext* compileTimeContext)
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator.release();
	}

	ASTHelper::updatePointerIfChanged(lhs, lhs->constCollapse(compileTimeContext));
	ASTHelper::updatePointerIfChanged(rhs, rhs->constCollapse(compileTimeContext));

	bool lhsIsConst = lhs->isConst();
	bool rhsIsConst = rhs->isConst();
	if (lhsIsConst && rhsIsConst)
	{
		Tokenizer::Lexeme collapsedLexeme = InfixOperatorOptimizer::combineLexemes(lhs, rhs);
		const CatGenericType lhsType = lhs->getType();
		if (lhsType.isBasicType())
		{
			return new CatLiteral(calculateExpression(compileTimeContext), getType(), collapsedLexeme);
		}
	}
	else
	{
		CatTypedExpression* collapsedExpression = InfixOperatorOptimizer::tryCollapseInfixOperator(lhs, rhs, oper);
		if (collapsedExpression != nullptr)
		{
			return collapsedExpression;
		}
	}
	return this;
}


std::any CatInfixOperator::execute(CatRuntimeContext* runtimeContext)
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator->execute(runtimeContext);
	}
	else
	{
		return calculateExpression(runtimeContext);
	}
}


bool CatInfixOperator::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	if (lhs->typeCheck(compiletimeContext, errorManager, errorContext)
		&& rhs->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		CatGenericType leftType = lhs->getType();
		CatGenericType rightType = rhs->getType();
		InfixOperatorResultInfo resultInfo = leftType.getInfixOperatorResultInfo(oper, rightType);
		resultType = resultInfo.getResultType();
		if (resultType.isValidType())
		{
			if (resultInfo.getIsOverloaded())
			{
				if (!resultInfo.getIsStaticOverloaded())
				{
					std::vector<CatTypedExpression*> arguments = {rhs.release()};
					overloadedOperator.reset(new CatMemberFunctionCall(::toString(oper), operatorLexeme, lhs.release(), new CatArgumentList(arguments[0]->getLexeme(), arguments), getLexeme()));
					return overloadedOperator->typeCheck(compiletimeContext, errorManager, errorContext);
				}
				else
				{
					std::vector<CatTypedExpression*> arguments = {lhs.release(), rhs.release()};
					overloadedOperator.reset(new CatStaticFunctionCall(new CatStaticScope(true, nullptr, resultInfo.getStaticOverloadedType()->getTypeName(), operatorLexeme, operatorLexeme), ::toString(oper), new CatArgumentList(arguments[0]->getLexeme(), arguments), getLexeme(),  operatorLexeme));
					return overloadedOperator->typeCheck(compiletimeContext, errorManager, errorContext);
				}
			}
			return true;
		}
		else if (resultInfo.getIsOverloaded())
		{
			errorManager->compiledWithError(Tools::append("Operator ", ::toString(oper), " not implemented for ", leftType.toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Invalid operation: ", leftType.toString(), " ", ::toString(oper), " ", rightType.toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	return false;
}


void CatInfixOperator::print() const
{
	CatLog::log("(");
	lhs->print();
	CatLog::log(" ");
	CatLog::log(toString(oper));
	CatLog::log(" ");
	rhs->print();
	CatLog::log(")");
}


CatTypedExpression* CatInfixOperator::getLeft() const
{
	return lhs.get();
}


 CatTypedExpression* CatInfixOperator::getRight() const
{
	return rhs.get();
}


CatInfixOperatorType CatInfixOperator::getOperatorType() const
{
	return oper;
}
