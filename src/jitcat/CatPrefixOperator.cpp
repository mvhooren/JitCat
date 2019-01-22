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

#include <cassert>

const char* CatPrefixOperator::conversionTable[] = {"!", "-"};


CatGenericType CatPrefixOperator::getType() const 
{
	if (oper == Operator::Not)
	{
		return CatGenericType::boolType;
	}
	else if (oper == Operator::Minus
			 && (rhs->getType().isFloatType()
			     || rhs->getType().isIntType()))
	{
		return rhs->getType();
	}
	else
	{
		return CatGenericType("Invalid use of operator.");
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
		return new CatLiteral(calculateExpression(compileTimeContext), getType());
	}
	return this;
}


std::any CatPrefixOperator::execute(CatRuntimeContext* runtimeContext)
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
			return CatGenericType::boolType;
		}
		else if (rightType.isFloatType()
				 && oper == Operator::Minus)
		{
			return CatGenericType::floatType;
		}
		else if (rightType.isIntType()
				 && oper == Operator::Minus)
		{
			return CatGenericType::intType;
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


inline std::any CatPrefixOperator::calculateExpression(CatRuntimeContext* runtimeContext)
{
	std::any rValue = rhs->execute(runtimeContext);
	if (rhs->getType().isBoolType()
		&& oper == Operator::Not)
	{
		return std::any(!std::any_cast<bool>(rValue));
	}
	else if (rhs->getType().isFloatType()
				&& oper == Operator::Minus)
	{
		return std::any(-std::any_cast<float>(rValue));
	}
	else if (rhs->getType().isIntType()
				&& oper == Operator::Minus)
	{
		return std::any(-std::any_cast<int>(rValue));
	}
	assert(false);
	return std::any();
}