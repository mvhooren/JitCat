/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Configuration.h"


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
		if (lType.isFloatType())
		{
			if		(rType.isIntType())			return calculateScalarExpression<float, int, float>(std::any_cast<float>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateScalarExpression<float, float, float>(std::any_cast<float>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateScalarExpression<float, double, double>(std::any_cast<float>(lValue), std::any_cast<double>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression<float, Configuration::CatString>(std::any_cast<float>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isDoubleType())
		{
			if		(rType.isIntType())			return calculateScalarExpression<double, int, double>(std::any_cast<double>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateScalarExpression<double, float, double>(std::any_cast<double>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateScalarExpression<double, double, double>(std::any_cast<double>(lValue), std::any_cast<double>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression<double, Configuration::CatString>(std::any_cast<double>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isIntType())
		{
			if		(rType.isIntType())			return calculateScalarExpression<int, int, int>(std::any_cast<int>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateScalarExpression<int, float, float>(std::any_cast<int>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateScalarExpression<int, double, double>(std::any_cast<int>(lValue), std::any_cast<double>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression<int, Configuration::CatString>(std::any_cast<int>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isStringValueType())
		{
			if		(rType.isIntType())			return calculateStringExpression<Configuration::CatString, int>(std::any_cast<Configuration::CatString>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateStringExpression<Configuration::CatString, float>(std::any_cast<Configuration::CatString>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateStringExpression<Configuration::CatString, double>(std::any_cast<Configuration::CatString>(lValue), std::any_cast<double>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression(std::any_cast<Configuration::CatString>(lValue), std::any_cast<Configuration::CatString>(rValue));
			else if (rType.isBoolType())		return calculateStringExpression(std::any_cast<Configuration::CatString>(lValue), std::any_cast<bool>(rValue));
		}
		else if (lType.isBoolType())
		{
			if		(rType.isBoolType())		return calculateBooleanExpression(std::any_cast<bool>(lValue), std::any_cast<bool>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression(std::any_cast<bool>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isPointerToReflectableObjectType() && rType.isPointerToReflectableObjectType())
		{
			switch (oper)
			{
				case CatInfixOperatorType::Equals:		return lType.getRawPointer(lValue) == rType.getRawPointer(rValue);
				case CatInfixOperatorType::NotEquals:	return lType.getRawPointer(lValue) != rType.getRawPointer(rValue);
				default:								assert(false);
			}
		}
		assert(false);
		return std::any();
	}
}


template<typename T, typename U, typename V>
inline std::any CatInfixOperator::calculateScalarExpression(const T& lValue, const U& rValue)
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
			if constexpr (jitcat::Configuration::divisionByZeroYieldsZero)
			{
				if (rValue != 0)
				{
					return std::any((V)lValue / (V)rValue);
				}
				else 
				{
					return std::any((V)0);
				}
			}
			else
			{
				return std::any((V)lValue / (V)rValue);
			}
		case CatInfixOperatorType::Modulo:
		{
			constexpr bool eitherIsFloat = std::is_same<T, float>::value 
										   || std::is_same<U, float>::value;
			constexpr bool eitherIsDouble = std::is_same<T, double>::value
											|| std::is_same<U, double>::value;
			if constexpr (eitherIsFloat || eitherIsDouble)
			{
				if constexpr (jitcat::Configuration::divisionByZeroYieldsZero)
				{
					if (rValue != 0)
					{
						if constexpr (eitherIsDouble)	return std::any((double)fmod((double)lValue, (double)rValue));
						else							return std::any((float)fmodf((float)lValue, (float)rValue));
					}
					else
					{
						if constexpr (eitherIsDouble)	return std::any(0.0);
						else							return std::any(0.0f);
					}
				}
				else
				{
					if constexpr (eitherIsDouble)	return std::any((double)fmod((double)lValue, (double)rValue));
					else							return std::any((float)fmodf((float)lValue, (float)rValue));
				}
			}
			else
			{
				if constexpr (jitcat::Configuration::divisionByZeroYieldsZero)
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
				else
				{
					return std::any((int)lValue % (int)rValue);
				}
			}
		}
		default:	assert(false);
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
			Configuration::CatStringStream stream;
			stream << lValue;
			stream << rValue;
			return std::any(stream.str());
		}
		default: assert(false);
	}
	assert(false);
	return std::any(Configuration::CatString());
}


inline std::any CatInfixOperator::calculateStringExpression(const Configuration::CatString& lValue, const Configuration::CatString& rValue)
{
	switch (oper)
	{
		case CatInfixOperatorType::Plus:		return std::any(lValue + rValue);
		case CatInfixOperatorType::Equals:		return std::any(lValue == rValue);
		case CatInfixOperatorType::NotEquals:	return std::any(lValue != rValue);
		default:								assert(false);
	}
	assert(false);
	return std::any(Configuration::CatString());
}


inline std::any CatInfixOperator::calculateBooleanExpression(bool lValue, bool rValue)
{
	switch (oper)
	{
		case CatInfixOperatorType::Equals:		return std::any(lValue == rValue);
		case CatInfixOperatorType::NotEquals:	return std::any(lValue != rValue);
		case CatInfixOperatorType::LogicalAnd:	return std::any(lValue && rValue);
		case CatInfixOperatorType::LogicalOr:	return std::any(lValue || rValue);
		default:								assert(false);
	}
	assert(false);
	return std::any(false);
}