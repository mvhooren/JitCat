/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatInfixOperator.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/InfixOperatorOptimizer.h"
#include "jitcat/ASTHelper.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


CatInfixOperator::CatInfixOperator(CatTypedExpression* lhs, CatTypedExpression* rhs, CatInfixOperatorType operatorType, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	rhs(rhs),
	lhs(lhs),
	oper(operatorType)
{
}


CatGenericType CatInfixOperator::getType() const
{
	const CatGenericType lhsType = lhs->getType();
	const CatGenericType rhsType = rhs->getType();

	switch (oper)
	{
		case CatInfixOperatorType::Plus:
			if ((lhsType.isStringType()
				 && (rhsType.isStringType()
				     || rhsType.isBasicType()))
				|| (lhsType.isBasicType()
					&& rhsType.isStringType()))
			{
				return CatGenericType::stringType;
			}
			//Intentional fall-through
		case CatInfixOperatorType::Minus:
		case CatInfixOperatorType::Multiply:
		case CatInfixOperatorType::Divide:
		case CatInfixOperatorType::Modulo:
			if (lhsType.isScalarType() && rhsType.isScalarType())
			{
				if (lhsType.isIntType() && rhsType.isIntType())
				{
					return CatGenericType::intType;
				}
				else
				{
					return CatGenericType::floatType;
				}
			}
			return CatGenericType("Expected scalar parameters.");
		case CatInfixOperatorType::Greater:
		case CatInfixOperatorType::Smaller:
		case CatInfixOperatorType::GreaterOrEqual:
		case CatInfixOperatorType::SmallerOrEqual:
			if (lhsType.isScalarType() && rhsType.isScalarType())
			{
				return CatGenericType::boolType;
			}
			return CatGenericType("Expected scalar parameters.");
		case CatInfixOperatorType::Equals:
		case CatInfixOperatorType::NotEquals:
			if ((lhsType.isScalarType() && rhsType.isScalarType())
				|| lhsType == rhsType)
			{
				return CatGenericType::boolType;
			}
			return CatGenericType("Parameters cannot be compared.");
		case CatInfixOperatorType::LogicalAnd:
		case CatInfixOperatorType::LogicalOr:
			if (lhsType.isBoolType()
				&& rhsType.isBoolType())
			{
				return CatGenericType::boolType;;
			}
			return CatGenericType("Expected boolean parameters.");
	}
	assert(false);
	return CatGenericType("Unexpected error.");
}


bool CatInfixOperator::isConst() const 
{
	return lhs->isConst() && rhs->isConst();
}


CatTypedExpression* CatInfixOperator::constCollapse(CatRuntimeContext* compileTimeContext)
{
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
	return calculateExpression(runtimeContext);
}


bool CatInfixOperator::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (lhs->typeCheck(compiletimeContext, errorManager, errorContext)
		&& rhs->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		CatGenericType leftType = lhs->getType();
		CatGenericType rightType = rhs->getType();
		resultType = leftType.getInfixOperatorResultType(oper, rightType);
		if (resultType != CatGenericType::errorType)
		{
			return true;
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
