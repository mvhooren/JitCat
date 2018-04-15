/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatPrefixOperator.h"
#include "CatLiteral.h"
#include "CatLog.h"
#include "OptimizationHelper.h"
#include "Tools.h"

const char* CatPrefixOperator::conversionTable[] = {"!", "-"};


CatGenericType CatPrefixOperator::getType() const 
{
	if (oper == Operator::Not)
	{
		return CatType::Bool;
	}
	else if (oper == Operator::Minus
			 && (rhs->getType() == CatType::Float
			     || rhs->getType() == CatType::Int))
	{
		return rhs->getType();
	}
	else
	{
		return CatType::Error;
	}
}


bool CatPrefixOperator::isConst() const 
{
	return rhs->isConst();
}


CatTypedExpression* CatPrefixOperator::constCollapse(CatRuntimeContext* compileTimeContext)
{
	OptimizationHelper::updatePointerIfChanged(rhs, rhs->constCollapse(compileTimeContext));
	if (rhs->isConst())
	{
		return new CatLiteral(calculateExpression(compileTimeContext));
	}
	return this;
}


CatValue CatPrefixOperator::execute(CatRuntimeContext* runtimeContext)
{
	return calculateExpression(runtimeContext);
}


CatGenericType CatPrefixOperator::typeCheck()
{
	CatGenericType rightType = rhs->typeCheck();
	if (!rightType.isValidType())
	{
		return rightType;
	}
	else
	{
		if (rightType.isBoolType()
			&& oper == Operator::Not)
		{
			return CatGenericType(CatType::Bool);
		}
		else if (rightType.isFloatType()
				 && oper == Operator::Minus)
		{
			return CatGenericType(CatType::Float);
		}
		else if (rightType.isIntType()
				 && oper == Operator::Minus)
		{
			return CatGenericType(CatType::Int);
		}
		else
		{
			return CatGenericType(Tools::append("Error: invalid operation: ", conversionTable[(unsigned int)oper], rightType.toString()));
		}
	}
}


void CatPrefixOperator::print() const
{
	CatLog::log("(");
	CatLog::log(conversionTable[(unsigned int)oper]);
	rhs->print();
	CatLog::log(")");
}


inline CatValue CatPrefixOperator::calculateExpression(CatRuntimeContext* runtimeContext)
{
	CatValue rValue = rhs->execute(runtimeContext);
	if (rValue.getValueType() == CatType::Bool
		&& oper == Operator::Not)
	{
		return CatValue(!rValue.getBoolValue());
	}
	else if (rhs->getType() == CatType::Float
				&& oper == Operator::Minus)
	{
		return CatValue(-rValue.getFloatValue());
	}
	else if (rhs->getType() == CatType::Int
				&& oper == Operator::Minus)
	{
		return CatValue(-rValue.getIntValue());
	}
	else
	{
		return CatValue(CatError(std::string("Error: invalid operation: ") + conversionTable[(unsigned int)oper] + toString(rhs->getType().getCatType()) ));
	}
}