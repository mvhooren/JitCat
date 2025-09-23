/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Configuration.h"



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