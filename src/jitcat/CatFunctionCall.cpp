/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatFunctionCall.h"
#include "CatArgumentList.h"
#include "CatLiteral.h"
#include "CatLog.h"
#include "JitCat.h"
#include "OptimizationHelper.h"
#include "Tools.h"

#include <algorithm>


CatFunctionCall::CatFunctionCall(const std::string& name, CatArgumentList* arguments):
	name(name),
	arguments(arguments)
{
	function = toFunction(name.c_str(), (int)arguments->arguments.size());
}


void CatFunctionCall::print() const
{
	CatLog::log(name);
	arguments->print();
}


CatASTNodeType CatFunctionCall::getNodeType()
{
	return CatASTNodeType::FunctionCall;
}


CatValue CatFunctionCall::execute(CatRuntimeContext* runtimeContext)
{
	std::size_t numArgumentsSupplied = arguments->arguments.size();
	if (function >= CatBuiltInFunctionType::Count)
	{
		return CatError(std::string("function not found: ") + name);
	}
	else if (checkArgumentCount(numArgumentsSupplied))
	{
		//At most 3 arguments, check for errors
		CatValue argumentValues[3];
		std::size_t numArgumentsToEvaluate = numArgumentsSupplied;
		if (function == CatBuiltInFunctionType::Select)
		{
			numArgumentsToEvaluate = 1;
		}
		for (unsigned int i = 0; i < numArgumentsToEvaluate && i < 3; i++)
		{
			argumentValues[i] = arguments->arguments[i]->execute(runtimeContext);
			if (argumentValues[i].getValueType() == CatType::Error)
			{
				return argumentValues[i];
			}
		}
		switch (function)
		{
			case CatBuiltInFunctionType::ToInt:		return CatValue(argumentValues[0].toIntValue());
			case CatBuiltInFunctionType::ToFloat:		return CatValue(argumentValues[0].toFloatValue());
			case CatBuiltInFunctionType::ToBool:		return CatValue(argumentValues[0].toBoolValue());
			case CatBuiltInFunctionType::ToString:		return CatValue(argumentValues[0].toStringValue());
			case CatBuiltInFunctionType::ToPrettyString:
			{
				if (argumentValues[0].getValueType() != CatType::Int)
				{
					return CatValue(argumentValues[0].toStringValue());
				}
				else
				{
					std::string numberString = argumentValues[0].toStringValue();
					size_t numberLength = numberString.length();
					int numParts = (((int)numberLength - 1) / 3) + 1;	// so that 1-3 results in 1, 4-6 in 2, etc
					std::string result = "";
					std::string separator = "";// to skip first space, cleaner result

					for (int i = 0; i < numParts; ++i)
					{
						int substringFirstIndex = (int)numberLength - (3 * (i + 1));
						int substringLength = 3;
						if (substringFirstIndex < 0)
						{
							// if only 2 digits are left, substringFirstIndex will be -1, and substringLength will need to be 2
							// if only 1 digit is left, substringFirstIndex is -2, and substringLength will need to be 1
							substringLength += substringFirstIndex;
							substringFirstIndex = 0;
						}
						result = numberString.substr((unsigned int)substringFirstIndex, (unsigned int)substringLength) + separator + result;
						separator = ",";
					}
					return CatValue(result);
				}
			}
			case CatBuiltInFunctionType::ToFixedLengthString:
			{
				if (argumentValues[0].getValueType() != CatType::Int
					|| argumentValues[1].getValueType() != CatType::Int)
				{
					return CatError("toFixedLengthString: expected an int.");
				}
				else
				{
					std::string numberString = argumentValues[0].toStringValue();
					while ((int)numberString.length() < argumentValues[1].getIntValue())
					{
						numberString = "0" + numberString;
					}
					return CatValue(numberString);
				}
			}
			case CatBuiltInFunctionType::Sin:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::sin(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("sin: expected a number as argument.");
				}
			case CatBuiltInFunctionType::Cos:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::cos(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("cos: expected a number as argument.");
				}
			case CatBuiltInFunctionType::Tan:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::tan(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("tan: expected a number as argument.");
				}
			case CatBuiltInFunctionType::Random:		return CatValue(static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX));
			case CatBuiltInFunctionType::RandomRange:
			{
				if (argumentValues[0].getValueType() == CatType::Bool
					&& argumentValues[1].getValueType() == CatType::Bool)
				{
					if (argumentValues[0].getBoolValue() != argumentValues[1].getBoolValue())
					{
						return (std::rand() % 2) == 1 ? CatValue(true) : CatValue(false);
					}
					else
					{
						return argumentValues[0];
					}
				}
				else if (argumentValues[0].getValueType() == CatType::Int
						 && argumentValues[1].getValueType() == CatType::Int)
				{
					int min = argumentValues[0].getIntValue();
					int max = argumentValues[1].getIntValue();
					if (min > max)
					{
						std::swap(min, max);
					}
					return CatValue(min + (std::rand() % (max - min + 1)));
				}
				else if (isScalar(argumentValues[0].getValueType())
						 && isScalar(argumentValues[1].getValueType()))
				{
					float min = argumentValues[0].toFloatValue();
					float max = argumentValues[1].toFloatValue();
					if (min > max)
					{
						std::swap(min, max);
					}
					float random = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
					return CatValue(min + random * (max - min));
				}
				else
				{
					return CatError("rand: invalid argument type.");
				}
			}
			case CatBuiltInFunctionType::Round:
				if (argumentValues[0].getValueType() == CatType::Float
					&& isScalar(argumentValues[1].getValueType()))
				{
					double multiplier = std::pow(10.0f, argumentValues[1].toIntValue());
					return CatValue((float)(std::floor(argumentValues[0].getFloatValue() * multiplier + 0.5f) / multiplier));
				}
				else
				{
					return CatError("round: can only round floating point numbers to integer number of decimals.");
				}
			case CatBuiltInFunctionType::StringRound:
				if (argumentValues[0].getValueType() == CatType::Float
					&& isScalar(argumentValues[1].getValueType()))
				{
					std::stringstream ss;
					ss.precision(argumentValues[1].toIntValue());
					ss.setf(std::ios_base::fixed);
					ss.unsetf(std::ios_base::scientific);
					ss << argumentValues[0].getFloatValue();
					std::string result = ss.str();
					int discardedCharacters = 0;
					if (result.find('.') != result.npos)
					{
						for (int i = (int)result.length() - 1; i >= 0; i--)
						{
							if (result[(unsigned int)i] == '0')
							{
								discardedCharacters++;
							}
							else if (result[(unsigned int)i] == '.')
							{
								discardedCharacters++;
								break;
							}
							else
							{
								break;
							}
						}
					}
					return CatValue(result.substr(0, result.length() - discardedCharacters));
				}
				else
				{
					return CatError("stringRound: can only round floating point numbers to integer number of decimals.");
				}
			case CatBuiltInFunctionType::Abs:
				if (argumentValues[0].getValueType() == CatType::Float)
				{
					return CatValue(std::abs(argumentValues[0].getFloatValue()));
				}
				else if (argumentValues[0].getValueType() == CatType::Int)
				{
					return CatValue(std::abs(argumentValues[0].getIntValue()));
				}
				else
				{
					return CatError("abs: expected a number as argument.");
				}
			case CatBuiltInFunctionType::Cap:
				if (isScalar(argumentValues[1].getValueType())
					&& isScalar(argumentValues[2].getValueType()))
				{
					if (argumentValues[0].getValueType() == CatType::Float)
					{
						float capValue = argumentValues[0].getFloatValue();
						float capMin = argumentValues[1].toFloatValue();
						float capMax = argumentValues[2].toFloatValue();
						if (capMin > capMax)
						{
							std::swap(capMin, capMax);
						}
						return std::max(capMin, std::min(capMax, capValue));
					}
					else if (argumentValues[0].getValueType() == CatType::Int)
					{
						int capValue = argumentValues[0].getIntValue();
						int capMin = argumentValues[1].toIntValue();
						int capMax = argumentValues[2].toIntValue();
						if (capMin > capMax)
						{
							std::swap(capMin, capMax);
						}
						return std::max(capMin, std::min(capMax, capValue));
					}
					else
					{
						return CatError("cap: value to be capped must be a number.");
					}
				}
				else
				{
					return CatError("cap: range must consist of 2 numbers.");
				}
			case CatBuiltInFunctionType::Min:
				if (argumentValues[0].getValueType() == CatType::Float
					&& isScalar(argumentValues[1].getValueType()))
				{
					return CatValue(std::min(argumentValues[0].getFloatValue(), argumentValues[1].toFloatValue()));
				}
				else if (argumentValues[0].getValueType() == CatType::Int
						 && isScalar(argumentValues[1].getValueType()))
				{
					return CatValue(std::min(argumentValues[0].getIntValue(), argumentValues[1].toIntValue()));
				}
				else
				{
					return CatError("min: expected two numbers as arguments.");
				}
			case CatBuiltInFunctionType::Max:
				if (argumentValues[0].getValueType() == CatType::Float
					&& isScalar(argumentValues[1].getValueType()))
				{
					return CatValue(std::max(argumentValues[0].getFloatValue(), argumentValues[1].toFloatValue()));
				}
				else if (argumentValues[0].getValueType() == CatType::Int
					&& isScalar(argumentValues[1].getValueType()))
				{
					return CatValue(std::max(argumentValues[0].getIntValue(), argumentValues[1].toIntValue()));
				}
				else
				{
					return CatError("max: expected two numbers as arguments.");
				}
			case CatBuiltInFunctionType::Log:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::log10(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("log: expected a number as argument.");
				}
			case CatBuiltInFunctionType::Sqrt:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::sqrt(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("sqrt: expected a number as argument.");
				}
			case CatBuiltInFunctionType::Pow:
				if (argumentValues[0].getValueType() == CatType::Float
					|| (argumentValues[1].getValueType() == CatType::Float 
						&& argumentValues[0].getValueType() == CatType::Int))
				{
					return CatValue(std::pow(argumentValues[0].toFloatValue(), argumentValues[1].toFloatValue()));
				}
				else if (argumentValues[0].getValueType() == CatType::Int
						 && argumentValues[1].getValueType() == CatType::Int)
				{
					return CatValue((int)std::pow((float)argumentValues[0].getIntValue(), (float)argumentValues[1].getIntValue()));
				}
				else
				{
					return CatError("pow: expected two numbers as arguments.");
				}
			case CatBuiltInFunctionType::Ceil:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::ceil(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("ceil: expected a number as argument.");
				}
			case CatBuiltInFunctionType::Floor:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::floor(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("floor: expected a number as argument.");
				}
			case CatBuiltInFunctionType::FindInString:
				if (isBasicType(argumentValues[0].getValueType())
					&& isBasicType(argumentValues[1].getValueType()))
				{
					std::string stringValue = argumentValues[0].toStringValue();
					std::string stringToFindValue = argumentValues[1].toStringValue();
					std::size_t pos = stringValue.find(stringToFindValue);
					int result = 0;
					if (pos == stringValue.npos)
					{
						result = -1;
					}
					else
					{
						result = (int)pos;
					}
					return CatValue(result);
				}
				else
				{
					return CatError("findInString: invalid argument.");
				}
			case CatBuiltInFunctionType::ReplaceInString:
				if (isBasicType(argumentValues[0].getValueType())
					&& isBasicType(argumentValues[1].getValueType())
					&& isBasicType(argumentValues[2].getValueType()))
				{
					std::string stringValue = argumentValues[0].toStringValue();
					std::string stringToFindValue = argumentValues[1].toStringValue();
					std::string replacementStringValue = argumentValues[2].toStringValue();
					if (stringToFindValue != "")
					{
						size_t startPosition = 0;
						while ((startPosition = stringValue.find(stringToFindValue, startPosition)) != std::string::npos)
						{
							stringValue.replace(startPosition, stringToFindValue.length(), replacementStringValue);
							startPosition += replacementStringValue.length(); 
						}
					}
					return CatValue(stringValue);
				}
				else
				{
					return CatError("replaceInString: invalid argument.");
				}
			case CatBuiltInFunctionType::StringLength:
				if (isBasicType(argumentValues[0].getValueType()))
				{
					return CatValue((int)argumentValues[0].toStringValue().size());
				}
				else
				{
					return CatError("stringLength: invalid argument.");
				}
			case CatBuiltInFunctionType::SubString:
				if (isBasicType(argumentValues[0].getValueType())
					&& isScalar(argumentValues[1].getValueType())
					&& isScalar(argumentValues[2].getValueType()))
				{
					std::string value = argumentValues[0].toStringValue();
					int offsetValue = argumentValues[1].toIntValue();
					if (value.size() == 0 && offsetValue == 0)
					{
						return CatValue("");
					}
					else if ((int)value.size() > offsetValue && offsetValue >= 0)
					{
						return CatValue(value.substr((unsigned int)offsetValue, (unsigned int)argumentValues[2].toIntValue()));
					}
					else
					{
						return CatError("subString: offset out of range.");
					}
				}
				else
				{
					return CatError("subString: invalid argument.");
				}
			case CatBuiltInFunctionType::Select:
			{
				if (argumentValues[0].getValueType() == CatType::Bool)
				{
					if (argumentValues[1].getValueType() == argumentValues[2].getValueType()
						|| (isScalar(argumentValues[1].getValueType()) && isScalar(argumentValues[2].getValueType())))
					{
						bool select = argumentValues[0].getBoolValue();
						if (select)
						{
							return arguments->arguments[1]->execute(runtimeContext);
						}
						else
						{
							return arguments->arguments[2]->execute(runtimeContext);
						}
					}
					else
					{
						return CatError("select: second and third argument must be the same type.");
					}
				}
				else
				{
					return CatError("select: first argument must resolve to a boolean.");
				}
			}
			default:
			case CatBuiltInFunctionType::Count:
			case CatBuiltInFunctionType::Invalid:
				return CatError(std::string("function not found: ") + name);
		}
	}
	else
	{
		std::stringstream stream;
		stream << "Invalid number of arguments in function: " << name << ".";
		return CatError(stream.str());
	}
}


CatGenericType CatFunctionCall::typeCheck()
{
	std::size_t numArgumentsSupplied = arguments->arguments.size();
	if (function >= CatBuiltInFunctionType::Count)
	{
		return CatGenericType(Tools::append("function not found: ", name));
	}
	else if (!checkArgumentCount(numArgumentsSupplied))
	{
		return CatGenericType(Tools::append("Invalid number of arguments in function: " , name));
	}
	else
	{
		CatGenericType argumentTypes[3];
		for (unsigned int i = 0; i < numArgumentsSupplied; i++)
		{
			argumentTypes[i] = arguments->arguments[i]->typeCheck();
			if (!argumentTypes[i].isValidType())
			{
				return argumentTypes[i];
			}
		}

		switch (function)
		{
			case CatBuiltInFunctionType::ToInt:			return CatGenericType(CatType::Int);
			case CatBuiltInFunctionType::ToFloat:			return CatGenericType(CatType::Float);
			case CatBuiltInFunctionType::ToBool:			return CatGenericType(CatType::Bool);
			case CatBuiltInFunctionType::ToString:			return CatGenericType(CatType::String);
			case CatBuiltInFunctionType::ToPrettyString:	return CatGenericType(CatType::String);
			case CatBuiltInFunctionType::ToFixedLengthString:
				if (argumentTypes[0].isIntType() && argumentTypes[1].isIntType())
				{
					return CatGenericType(CatType::String);
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected an int."));
				}
			case CatBuiltInFunctionType::Sin:
			case CatBuiltInFunctionType::Cos:
			case CatBuiltInFunctionType::Tan:
				if (argumentTypes[0].isScalarType())
				{
					return CatGenericType(CatType::Float);
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected a number as argument."));
				}
			case CatBuiltInFunctionType::Random:		return CatGenericType(CatType::Float);
			case CatBuiltInFunctionType::RandomRange:
			{
				if (argumentTypes[0].isBoolType()
					&& argumentTypes[1].isBoolType())
				{
					return CatGenericType(CatType::Bool);
				}
				else if (argumentTypes[0].isIntType()
					&& argumentTypes[1].isScalarType())
				{
					return CatGenericType(CatType::Int);
				}
				else if (argumentTypes[0].isFloatType()
					&& argumentTypes[0].isScalarType())
				{
					return CatGenericType(CatType::Float);
				}
				else
				{
					return CatGenericType(Tools::append(name, ": invalid argument types."));
				}
			}
			case CatBuiltInFunctionType::Round:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					return CatGenericType(CatType::Float);
				}
				else
				{
					return CatGenericType("round: can only round floating point numbers to integer number of decimals.");
				}
			case CatBuiltInFunctionType::StringRound:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					return CatGenericType(CatType::String);
				}
				else
				{
					return CatGenericType("stringRound: can only round floating point numbers to integer number of decimals.");
				}
			case CatBuiltInFunctionType::Abs:
				if (argumentTypes[0].isFloatType())
				{
					return CatGenericType(CatType::Float);
				}
				else if (argumentTypes[0].isIntType())
				{
					return CatGenericType(CatType::Int);
				}
				else
				{
					return CatGenericType("abs: expected a number as argument.");
				}
			case CatBuiltInFunctionType::Cap:
				if (argumentTypes[1].isScalarType()
					&& argumentTypes[2].isScalarType())
				{
					if (argumentTypes[0].isFloatType())
					{
						return CatGenericType(CatType::Float);
					}
					else if (argumentTypes[0].isIntType())
					{
						return CatGenericType(CatType::Int);
					}
					else
					{
						return CatGenericType("cap: value to be capped must be a number.");
					}
				}
				else
				{
					return CatGenericType("cap: range must consist of 2 numbers.");
				}
			case CatBuiltInFunctionType::Min:
			case CatBuiltInFunctionType::Max:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					return CatGenericType(CatType::Float);
				}
				else if (argumentTypes[0].isIntType()
						 && argumentTypes[1].isScalarType())
				{
					return CatGenericType(CatType::Int);
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected two numbers as arguments."));
				}
			case CatBuiltInFunctionType::Log:
			case CatBuiltInFunctionType::Sqrt:
			case CatBuiltInFunctionType::Ceil:
			case CatBuiltInFunctionType::Floor:
				if (argumentTypes[0].isScalarType())
				{
					return CatGenericType(CatType::Float);
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected a number as argument."));
				}
			case CatBuiltInFunctionType::Pow:
				if (argumentTypes[0].isFloatType()
					|| (argumentTypes[1].isFloatType()
						&& argumentTypes[0].isIntType()))
				{
					return CatGenericType(CatType::Float);
				}
				else if (argumentTypes[0].isIntType()
						 && argumentTypes[1].isIntType())
				{
					return CatGenericType(CatType::Int);
				}
				else
				{
					return CatGenericType("pow: expected two numbers as arguments.");
				}
			case CatBuiltInFunctionType::FindInString:
				if (argumentTypes[0].isBasicType()
					&& argumentTypes[1].isBasicType())
				{
					return CatGenericType(CatType::Int);
				}
				else
				{
					return CatGenericType("findInString: invalid argument.");
				}
			case CatBuiltInFunctionType::ReplaceInString:
				if (argumentTypes[0].isBasicType()
					&& argumentTypes[1].isBasicType()
					&& argumentTypes[2].isBasicType())
				{
					return CatGenericType(CatType::String);
				}
				else
				{
					return CatGenericType("replaceInString: invalid argument.");
				}
			case CatBuiltInFunctionType::StringLength:
				if (argumentTypes[0].isBasicType())
				{
					return CatGenericType(CatType::Int);
				}
				else
				{
					return CatGenericType("stringLength: invalid argument.");
				}
			case CatBuiltInFunctionType::SubString:
				if (argumentTypes[0].isBasicType()
					&& argumentTypes[1].isScalarType()
					&& argumentTypes[2].isScalarType())
				{
					return CatGenericType(CatType::String);
				}
				else
				{
					return CatGenericType("subString: invalid argument.");
				}
			case CatBuiltInFunctionType::Select:
			{
				if (argumentTypes[0].isBoolType())
				{
					if (argumentTypes[1] == argumentTypes[2]
						|| (argumentTypes[1].isScalarType() && argumentTypes[2].isScalarType()))
					{
						return argumentTypes[1];
					}
					else
					{
						return CatGenericType("select: second and third argument must be the same type.");
					}
				}
				else
				{
					return CatGenericType("select: first argument must resolve to a boolean.");
				}
			}
			default:
			case CatBuiltInFunctionType::Count:
			case CatBuiltInFunctionType::Invalid:
				return CatGenericType(std::string("function not found: ") + name);
		}
	}
}


CatGenericType CatFunctionCall::getType() const
{
	if (checkArgumentCount(arguments->arguments.size()))
	{
		switch (function)
		{
			default:
				return CatType::Error;
			case CatBuiltInFunctionType::ToInt:
			case CatBuiltInFunctionType::StringLength:
			case CatBuiltInFunctionType::Round:
			case CatBuiltInFunctionType::FindInString:
				return CatType::Int;
			case CatBuiltInFunctionType::ToBool:
				return CatType::Bool;
			case CatBuiltInFunctionType::RandomRange:
			case CatBuiltInFunctionType::Abs:
			case CatBuiltInFunctionType::Cap:
			case CatBuiltInFunctionType::Min:
			case CatBuiltInFunctionType::Max:
				return arguments->arguments[0]->getType();
			case CatBuiltInFunctionType::Select:
				return arguments->arguments[1]->getType();
			case CatBuiltInFunctionType::ToFloat:
			case CatBuiltInFunctionType::Sin:
			case CatBuiltInFunctionType::Cos:
			case CatBuiltInFunctionType::Tan:
			case CatBuiltInFunctionType::Random:
			case CatBuiltInFunctionType::Log:
			case CatBuiltInFunctionType::Sqrt:
			case CatBuiltInFunctionType::Ceil:
			case CatBuiltInFunctionType::Floor:
				return CatType::Float;
			case CatBuiltInFunctionType::SubString:
			case CatBuiltInFunctionType::ToString:
			case CatBuiltInFunctionType::ToPrettyString:
			case CatBuiltInFunctionType::ToFixedLengthString:
			case CatBuiltInFunctionType::StringRound:
			case CatBuiltInFunctionType::ReplaceInString:
				return CatType::String;
			case CatBuiltInFunctionType::Pow:
				return arguments->arguments[0]->getType() == CatType::Int ? arguments->arguments[1]->getType() : arguments->arguments[0]->getType();
		}
	}
	else
	{
		return CatType::Error;
	}
}


bool CatFunctionCall::isConst() const
{
	if (isDeterministic())
	{
		std::vector<std::unique_ptr<CatTypedExpression>>& argumentList = arguments->arguments;
		std::size_t numArguments = argumentList.size();
		for (std::size_t i = 0; i < numArguments; i++)
		{
			if (!argumentList[i]->isConst())
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}


CatTypedExpression* CatFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext)
{
	bool allArgumentsAreConst = true;
	for (auto& iter : arguments->arguments)
	{
		OptimizationHelper::updatePointerIfChanged(iter, iter->constCollapse(compileTimeContext));
		if (!iter->isConst())
		{
			allArgumentsAreConst = false;
		}
	}
	if (isDeterministic() && allArgumentsAreConst)
	{
		return new CatLiteral(execute(compileTimeContext));
	}
	else if (function == CatBuiltInFunctionType::Select
			 && arguments->arguments[0]->isConst())
	{
		bool value = arguments->arguments[0]->execute(compileTimeContext).toBoolValue();
		if (value)
		{
			return arguments->arguments[1].release();
		}
		else
		{
			return arguments->arguments[2].release();
		}
	}
	return this;
}


CatBuiltInFunctionType CatFunctionCall::getFunctionType() const
{
	return function;
}


const std::string& CatFunctionCall::getFunctionName() const
{
	return name;
}


CatArgumentList* CatFunctionCall::getArgumentList() const
{
	return arguments.get();
}


bool CatFunctionCall::isBuiltInFunction(const char* functionName, int numArguments)
{
	return toFunction(functionName, numArguments) != CatBuiltInFunctionType::Invalid;
}


const std::vector<std::string>& CatFunctionCall::getAllBuiltInFunctions()
{
	return functionTable;
}


bool CatFunctionCall::isDeterministic() const
{
	return function != CatBuiltInFunctionType::Random 
		   && function != CatBuiltInFunctionType::RandomRange;
}


bool CatFunctionCall::checkArgumentCount(std::size_t count) const
{
	switch (function)
	{
		default:
		case CatBuiltInFunctionType::Random:
			return count == 0;
		case CatBuiltInFunctionType::ToInt:
		case CatBuiltInFunctionType::ToString:
		case CatBuiltInFunctionType::ToPrettyString:
		case CatBuiltInFunctionType::StringLength:
		case CatBuiltInFunctionType::ToBool:
		case CatBuiltInFunctionType::Abs:
		case CatBuiltInFunctionType::ToFloat:
		case CatBuiltInFunctionType::Sin:
		case CatBuiltInFunctionType::Cos:
		case CatBuiltInFunctionType::Tan:
		case CatBuiltInFunctionType::Log:
		case CatBuiltInFunctionType::Sqrt:
		case CatBuiltInFunctionType::Ceil:
		case CatBuiltInFunctionType::Floor:
			return count == 1;
		case CatBuiltInFunctionType::Round:
		case CatBuiltInFunctionType::RandomRange:
		case CatBuiltInFunctionType::StringRound:
		case CatBuiltInFunctionType::Min:
		case CatBuiltInFunctionType::Max:
		case CatBuiltInFunctionType::Pow:
		case CatBuiltInFunctionType::FindInString:
		case CatBuiltInFunctionType::ToFixedLengthString:
			return count == 2;
		case CatBuiltInFunctionType::Cap:
		case CatBuiltInFunctionType::SubString:
		case CatBuiltInFunctionType::ReplaceInString:
		case CatBuiltInFunctionType::Select:
			return count == 3;
	}
}


CatBuiltInFunctionType CatFunctionCall::toFunction(const char* functionName, int numArguments)
{
	for (unsigned int i = 0; i < (unsigned int)CatBuiltInFunctionType::Count; i++)
	{
		if (Tools::equalsWhileIgnoringCase(functionTable[i], functionName))
		{
			CatBuiltInFunctionType functionType = (CatBuiltInFunctionType)i;
			if (functionType == CatBuiltInFunctionType::Random)
			{
				if (numArguments == 2)
				{
					return CatBuiltInFunctionType::RandomRange;
				}
			}
			return functionType;
		}
	}
	return CatBuiltInFunctionType::Invalid;
}


std::vector<std::string> CatFunctionCall::functionTable = 	
{
	 "toInt",				//CatBuiltInFunctionType::ToInt
	 "toFloat",				//CatBuiltInFunctionType::ToFloat
	 "toBool",				//CatBuiltInFunctionType::ToBool
	 "toString",			//CatBuiltInFunctionType::ToString
	 "toPrettyString",		//CatBuiltInFunctionType::ToPrettyString
	 "toFixedLengthString",	//CatBuiltInFunctionType::ToFixedLengthString
	 "sin",					//CatBuiltInFunctionType::Sin
	 "cos",					//CatBuiltInFunctionType::Cos
	 "tan",					//CatBuiltInFunctionType::Tan
	 "rand",				//CatBuiltInFunctionType::Random
	 "rand",				//CatBuiltInFunctionType::RandomRange
	 "round",				//CatBuiltInFunctionType::Round
	 "stringRound",			//CatBuiltInFunctionType::StringRound
	 "abs",					//CatBuiltInFunctionType::Abs
	 "cap",					//CatBuiltInFunctionType::Cap
	 "min",					//CatBuiltInFunctionType::Min 
	 "max",					//CatBuiltInFunctionType::Max
	 "log",					//CatBuiltInFunctionType::Log
	 "sqrt",				//CatBuiltInFunctionType::Sqrt
	 "pow",					//CatBuiltInFunctionType::Pow
	 "ceil",				//CatBuiltInFunctionType::Ceil
	 "floor",				//CatBuiltInFunctionType::Floor
	 "findInString",		//CatBuiltInFunctionType::FindInString,
	 "replaceInString",		//CatBuiltInFunctionType::ReplaceInString,
	 "stringLength",		//CatBuiltInFunctionType::StringLength
	 "subString",			//CatBuiltInFunctionType::SubString
	 "select"				//CatBuiltInFunctionType::Select
};