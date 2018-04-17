/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatInfixOperator.h"
#include "CatLiteral.h"
#include "CatLog.h"
#include "InfixOperatorOptimizer.h"
#include "OptimizationHelper.h"


CatGenericType CatInfixOperator::getType() const 
{
	CatType lhsType = lhs->getType().getCatType();
	if (lhsType == CatType::Error)
	{
		return CatType::Error;
	}
	CatType rhsType = rhs->getType().getCatType();
	if (rhsType == CatType::Error)
	{
		return CatType::Error;
	}

	switch (oper)
	{
		case CatInfixOperatorType::Assign:
			return CatType::Void;
		case CatInfixOperatorType::Plus:
			if ((lhsType == CatType::String
				 && (rhsType == CatType::String
				     || isScalar(rhsType)))
					 || (isScalar(lhsType) 
					     && rhsType == CatType::String))
			{
				return CatType::String;
			}
		case CatInfixOperatorType::Minus:
		case CatInfixOperatorType::Multiply:
		case CatInfixOperatorType::Divide:
			if (isScalar(lhsType) && isScalar(rhsType))
			{
				if (lhsType == CatType::Int && rhsType == CatType::Int)
				{
					return CatType::Int;
				}
				else
				{
					return CatType::Float;
				}
			}
			return CatType::Error;
		case CatInfixOperatorType::Modulo:
			if (isScalar(lhsType) && isScalar(rhsType))
			{
				return CatType::Int;
			}
			return CatType::Error;
		case CatInfixOperatorType::Greater:
		case CatInfixOperatorType::Smaller:
		case CatInfixOperatorType::GreaterOrEqual:
		case CatInfixOperatorType::SmallerOrEqual:
			if (isScalar(lhsType) && isScalar(rhsType))
			{
				return CatType::Bool;
			}
			return CatType::Error;
		case CatInfixOperatorType::Equals:
		case CatInfixOperatorType::NotEquals:
			if ((isScalar(lhsType) && isScalar(rhsType))
				|| lhsType == rhsType)
			{
				return CatType::Bool;
			}
			return CatType::Error;
		case CatInfixOperatorType::LogicalAnd:
		case CatInfixOperatorType::LogicalOr:
			if (lhsType == CatType::Bool
				&& rhsType == CatType::Bool)
			{
				return CatType::Bool;
			}
			return CatType::Error;
	}
	return CatType::Error;
}


bool CatInfixOperator::isConst() const 
{
	if (oper == CatInfixOperatorType::Assign)
	{
		return false;
	}
	else
	{
		return lhs->isConst() && rhs->isConst();
	}
}


CatTypedExpression* CatInfixOperator::constCollapse(CatRuntimeContext* compileTimeContext)
{
	OptimizationHelper::updatePointerIfChanged(lhs, lhs->constCollapse(compileTimeContext));
	OptimizationHelper::updatePointerIfChanged(rhs, rhs->constCollapse(compileTimeContext));

	bool lhsIsConst = lhs->isConst();
	bool rhsIsConst = rhs->isConst();
	if (lhsIsConst && rhsIsConst)
	{
		CatType lhsType = lhs->getType().getCatType();
		if ((   lhsType == CatType::Int
			 || lhsType == CatType::Float
			 || lhsType == CatType::String
			 || lhsType == CatType::Bool)
			&& oper != CatInfixOperatorType::Assign)
		{
			return new CatLiteral(calculateExpression(compileTimeContext));
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


CatValue CatInfixOperator::execute(CatRuntimeContext* runtimeContext)
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