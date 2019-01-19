/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatRuntimeContext.h"

#include <cassert>
#include <cmath>
#include <string>
#include <sstream>


inline std::any CatInfixOperator::calculateExpression(CatRuntimeContext* runtimeContext)
{
	//Optimization to prevent always calculating both sides of an && or an ||
	if (oper == CatInfixOperatorType::LogicalAnd || oper == CatInfixOperatorType::LogicalOr)
	{
		std::any lValue = lhs->execute(runtimeContext);
		//If lValue is true and oper is || then result is true
		//If lValue is false and oper is && then result is false
		if (std::any_cast<bool>(lValue) == (oper == CatInfixOperatorType::LogicalOr))
		{
			return lValue;
		}
		else
		{
			std::any rValue = rhs->execute(runtimeContext);
			if (oper == CatInfixOperatorType::LogicalAnd)
			{
				return std::any(std::any_cast<bool>(lValue) && std::any_cast<bool>(rValue));
			}
			else
			{
				return std::any(std::any_cast<bool>(lValue) || std::any_cast<bool>(rValue));
			}
		}
	}
	else
	{

		std::any lValue = lhs->execute(runtimeContext);
		std::any rValue = rhs->execute(runtimeContext);
		CatGenericType lType = lhs->getType();
		CatGenericType rType = rhs->getType();
		bool allowDivisionByZero = runtimeContext == nullptr || !runtimeContext->isRunTimeContext();
		switch (lType.getCatType())
		{
			case CatType::Float:
				switch (rType.getCatType())
				{
					case CatType::Int:		return calculateScalarExpression<float, int, float>(std::any_cast<float>(lValue), std::any_cast<int>(rValue), allowDivisionByZero);
					case CatType::Float:	return calculateScalarExpression<float, float, float>(std::any_cast<float>(lValue), std::any_cast<float>(rValue), allowDivisionByZero);
					case CatType::String:	return calculateStringExpression<float, std::string>(std::any_cast<float>(lValue), std::any_cast<std::string>(rValue));
				}
				break;
			case CatType::Int:
				switch (rType.getCatType())
				{
					case CatType::Int:		return calculateScalarExpression<int, int, int>(std::any_cast<int>(lValue), std::any_cast<int>(rValue), allowDivisionByZero);
					case CatType::Float:	return calculateScalarExpression<int, float, float>(std::any_cast<int>(lValue), std::any_cast<float>(rValue), allowDivisionByZero);
					case CatType::String:	return calculateStringExpression<int, std::string>(std::any_cast<int>(lValue), std::any_cast<std::string>(rValue));
				}
				break;
			case CatType::String:
				switch (rType.getCatType())
				{
					case CatType::Int:		return calculateStringExpression<std::string, int>(std::any_cast<std::string>(lValue), std::any_cast<int>(rValue));
					case CatType::Float:	return calculateStringExpression<std::string, float>(std::any_cast<std::string>(lValue), std::any_cast<float>(rValue));
					case CatType::String:	return calculateStringExpression(std::any_cast<std::string>(lValue), std::any_cast<std::string>(rValue));
					case CatType::Bool:	return calculateStringExpression(std::any_cast<std::string>(lValue), std::any_cast<bool>(rValue));
				}
				break;
			case CatType::Bool:
				switch (rType.getCatType())
				{
					case CatType::Bool:	return calculateBooleanExpression(std::any_cast<bool>(lValue), std::any_cast<bool>(rValue));
					case CatType::String:	return calculateStringExpression(std::any_cast<bool>(lValue), std::any_cast<std::string>(rValue));
				}
				break;
		}
		assert(false);
		return std::any();
	}
}


template<typename T, typename U, typename V>
inline std::any CatInfixOperator::calculateScalarExpression(const T& lValue, const U& rValue, bool allowDivideByZero)
{
	switch (oper)
	{
		case CatInfixOperatorType::Plus:			return std::any((V)lValue + (V)rValue);
		case CatInfixOperatorType::Minus:			return std::any((V)lValue - (V)rValue);
		case CatInfixOperatorType::Multiply:		return std::any((V)lValue * (V)rValue);
		case CatInfixOperatorType::Greater:			return std::any(lValue > rValue);
		case CatInfixOperatorType::Smaller:			return std::any(lValue < rValue);
		case CatInfixOperatorType::GreaterOrEqual:	return std::any(lValue >= rValue);
		case CatInfixOperatorType::SmallerOrEqual:	return std::any(lValue <= rValue);
		case CatInfixOperatorType::Equals:			return std::any(lValue == rValue);
		case CatInfixOperatorType::NotEquals:		return std::any(lValue != rValue);
		case CatInfixOperatorType::Divide:
			if (rValue != 0)
			{
				return std::any((V)lValue / (V)rValue);
			}
			else 
			{
				return std::any((V)0);
			}
		case CatInfixOperatorType::Modulo:
			if constexpr (std::is_same<T, float>::value || std::is_same<U, float>::value)
			{
				if ((float)rValue != 0)
				{
					return std::any((float)fmodf((float)lValue, (float)rValue));
				}
				else
				{
					return std::any(0.0f);
				}
			}
			else
			{
				if ((int)rValue != 0)
				{
					return std::any((int)lValue % (int)rValue);
				}
				else
				{
					return std::any(0);
				}
			}
	}
	assert(false);
	return std::any(V());
}


template<typename T, typename U>
inline std::any CatInfixOperator::calculateStringExpression(const T& lValue, const U& rValue)
{
	switch (oper)
	{
		case CatInfixOperatorType::Plus:
		{
			std::stringstream stream;
			stream << lValue;
			stream << rValue;
			return std::any(stream.str());
		}
	}
	assert(false);
	return std::any(std::string());
}


inline std::any CatInfixOperator::calculateStringExpression(const std::string& lValue, const std::string& rValue)
{
	switch (oper)
	{
		case CatInfixOperatorType::Plus:		return std::any(lValue + rValue);
		case CatInfixOperatorType::Equals:		return std::any(lValue == rValue);
		case CatInfixOperatorType::NotEquals:	return std::any(lValue != rValue);
	}
	assert(false);
	return std::any(std::string());
}


inline std::any CatInfixOperator::calculateBooleanExpression(bool lValue, bool rValue)
{
	switch (oper)
	{
		case CatInfixOperatorType::Equals:		return std::any(lValue == rValue);
		case CatInfixOperatorType::NotEquals:	return std::any(lValue != rValue);
		case CatInfixOperatorType::LogicalAnd:	return std::any(lValue && rValue);
		case CatInfixOperatorType::LogicalOr:	return std::any(lValue || rValue);
	}
	assert(false);
	return std::any(false);
}