/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatInfixOperator.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/InfixOperatorOptimizer.h"
#include "jitcat/ASTHelper.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


CatInfixOperator::CatInfixOperator(CatTypedExpression* lhs, CatTypedExpression* rhs, CatInfixOperatorType operatorType):
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
				     || rhsType.isScalarType()))
				|| (lhsType.isScalarType()
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
		const CatGenericType lhsType = lhs->getType();
		if (lhsType.isBasicType())
		{
			return new CatLiteral(calculateExpression(compileTimeContext), getType());
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


CatGenericType CatInfixOperator::typeCheck()
{
	CatGenericType leftType = lhs->typeCheck();
	CatGenericType rightType = rhs->typeCheck();
	return leftType.getInfixOperatorResultType(oper, rightType);
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
