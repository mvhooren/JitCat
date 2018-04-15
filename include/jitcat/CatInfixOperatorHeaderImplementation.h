/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatRuntimeContext.h"

#include <string>
#include <sstream>


inline CatValue CatInfixOperator::calculateExpression(CatRuntimeContext* runtimeContext)
{
	CatExpression* lExpression= static_cast<CatExpression*>(lhs.get());
	CatExpression* rExpression = static_cast<CatExpression*>(rhs.get());

	//Optimization to prevent always calculating both sides of an && or an ||
	if (oper == CatInfixOperatorType::LogicalAnd || oper == CatInfixOperatorType::LogicalOr)
	{
		CatValue lValue = lExpression->execute(runtimeContext);
		CatType lType = lValue.getValueType();
		if (lType == CatType::Error)
		{
			return CatValue(lValue.getErrorValue());
		}
		//If lValue is true and oper is || then result is true
		//If lValue is false and oper is && then result is false
		if (lValue.getBoolValue() == (oper == CatInfixOperatorType::LogicalOr))
		{
			return lValue;
		}
		else
		{
			CatValue rValue = rExpression->execute(runtimeContext);
			CatType rType = rValue.getValueType();
			if (rType == CatType::Error)
			{
				return CatValue(rValue.getErrorValue());
			}
			if (oper == CatInfixOperatorType::LogicalAnd)
			{
				return CatValue(lValue.getBoolValue() && rValue.getBoolValue());
			}
			else
			{
				return CatValue(lValue.getBoolValue() || rValue.getBoolValue());
			}
		}
	}
	else
	{

		CatValue lValue = lExpression->execute(runtimeContext);
		CatValue rValue = rExpression->execute(runtimeContext);
		CatType lType = lValue.getValueType();
		CatType rType = rValue.getValueType();
		if (rType == CatType::Error)
		{
			return CatValue(rValue.getErrorValue());
		}
		bool allowDivisionByZero = runtimeContext == nullptr || !runtimeContext->isRunTimeContext();
		switch (lType)
		{
			case CatType::Error:			return CatValue(lValue.getErrorValue());
			case CatType::Float:
				switch (rType)
				{
					case CatType::Int:		return calculateScalarExpression<float, int, float>(lValue.getFloatValue(), rValue.getIntValue(), allowDivisionByZero);
					case CatType::Float:	return calculateScalarExpression<float, float, float>(lValue.getFloatValue(), rValue.getFloatValue(), allowDivisionByZero);
					case CatType::String:	return calculateStringExpression<float, std::string>(lValue.getFloatValue(), rValue.getStringValue());
				}
				break;
			case CatType::Int:
				switch (rType)
				{
					case CatType::Int:		return calculateScalarExpression<int, int, int>(lValue.getIntValue(), rValue.getIntValue(), allowDivisionByZero);
					case CatType::Float:	return calculateScalarExpression<int, float, float>(lValue.getIntValue(), rValue.getFloatValue(), allowDivisionByZero);
					case CatType::String:	return calculateStringExpression<int, std::string>(lValue.getIntValue(), rValue.getStringValue());
				}
				break;
			case CatType::String:
				switch (rType)
				{
					case CatType::Int:		return calculateStringExpression<std::string, int>(lValue.getStringValue(), rValue.getIntValue());
					case CatType::Float:	return calculateStringExpression<std::string, float>(lValue.getStringValue(), rValue.getFloatValue());
					case CatType::String:	return calculateStringExpression(lValue.getStringValue(), rValue.getStringValue());
					case CatType::Bool:	return calculateStringExpression(lValue.getStringValue(), rValue.getBoolValue());
				}
				break;
			case CatType::Bool:
				switch (rType)
				{
					case CatType::Bool:	return calculateBooleanExpression(lValue.getBoolValue(), rValue.getBoolValue());
					case CatType::String:	return calculateStringExpression(lValue.getBoolValue(), rValue.getStringValue());
				}
				break;
		}
		return CatValue(CatError(std::string("Invalid operation: ") + toString(lType) + " " + toString(oper) + " " + toString(rType)));
	}
}


template<typename T, typename U, typename V>
inline CatValue CatInfixOperator::calculateScalarExpression(const T& lValue, const U& rValue, bool allowDivideByZero)
{
	switch (oper)
	{
		case CatInfixOperatorType::Plus:			return CatValue((V)lValue + (V)rValue);
		case CatInfixOperatorType::Minus:			return CatValue((V)lValue - (V)rValue);
		case CatInfixOperatorType::Multiply:		return CatValue((V)lValue * (V)rValue);
		case CatInfixOperatorType::Greater:			return CatValue(lValue > rValue);
		case CatInfixOperatorType::Smaller:			return CatValue(lValue < rValue);
		case CatInfixOperatorType::GreaterOrEqual:	return CatValue(lValue >= rValue);
		case CatInfixOperatorType::SmallerOrEqual:	return CatValue(lValue <= rValue);
		case CatInfixOperatorType::Equals:			return CatValue(lValue == rValue);
		case CatInfixOperatorType::NotEquals:		return CatValue(lValue != rValue);
		case CatInfixOperatorType::Divide:
			if (rValue != 0)
			{
				return CatValue((V)lValue / (V)rValue);
			}
			else if (allowDivideByZero)
			{
				return CatValue((V)0);
			}
			else
			{
				return CatValue(CatError("Division by zero."));
			}
		case CatInfixOperatorType::Modulo:
			if ((int)rValue != 0)
			{
				return CatValue((int)lValue % (int)rValue);
			}
			else
			{
				return lValue;
			}
	}
	return CatValue(CatError(std::string("Invalid operation: ") + lhs->getType().toString() + " " + toString(oper) + " " + rhs->getType().toString()));
}


template<typename T, typename U>
inline CatValue CatInfixOperator::calculateStringExpression(const T& lValue, const U& rValue)
{

	switch (oper)
	{
		case CatInfixOperatorType::Plus:
		{
			std::stringstream stream;
			stream << lValue;
			stream << rValue;
			return CatValue(stream.str());
		}
	}
	return CatValue(CatError(std::string("Invalid operation: ") + lhs->getType().toString() + " " + toString(oper) + " " + rhs->getType().toString()));
}


inline CatValue CatInfixOperator::calculateStringExpression(const std::string& lValue, const std::string& rValue)
{
	switch (oper)
	{
		case CatInfixOperatorType::Plus:		return CatValue(lValue + rValue);
		case CatInfixOperatorType::Equals:		return CatValue(lValue == rValue);
		case CatInfixOperatorType::NotEquals:	return CatValue(lValue != rValue);
	}
	return CatValue(CatError(std::string("Invalid operation: ") + lhs->getType().toString() + " " + toString(oper) + " " + rhs->getType().toString()));
}


inline CatValue CatInfixOperator::calculateBooleanExpression(bool lValue, bool rValue)
{
	switch (oper)
	{
		case CatInfixOperatorType::Equals:		return CatValue(lValue == rValue);
		case CatInfixOperatorType::NotEquals:	return CatValue(lValue != rValue);
		case CatInfixOperatorType::LogicalAnd:	return CatValue(lValue && rValue);
		case CatInfixOperatorType::LogicalOr:	return CatValue(lValue || rValue);
	}
	return CatValue(CatError(std::string("Invalid operation: ") + lhs->getType().toString() + " " + toString(oper) + " " + rhs->getType().toString()));
}