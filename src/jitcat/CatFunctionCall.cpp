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
	if (function >= BuiltInFunction::Count)
	{
		return CatError(std::string("function not found: ") + name);
	}
	else if (checkArgumentCount(numArgumentsSupplied))
	{
		//At most 3 arguments, check for errors
		CatValue argumentValues[3];
		std::size_t numArgumentsToEvaluate = numArgumentsSupplied;
		if (function == BuiltInFunction::Select)
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
			case BuiltInFunction::ToInt:		return CatValue(argumentValues[0].toIntValue());
			case BuiltInFunction::ToFloat:		return CatValue(argumentValues[0].toFloatValue());
			case BuiltInFunction::ToBool:		return CatValue(argumentValues[0].toBoolValue());
			case BuiltInFunction::ToString:		return CatValue(argumentValues[0].toStringValue());
			case BuiltInFunction::ToPrettyString:
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
			case BuiltInFunction::ToFixedLengthString:
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
			case BuiltInFunction::Sin:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::sin(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("sin: expected a number as argument.");
				}
			case BuiltInFunction::Cos:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::cos(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("cos: expected a number as argument.");
				}
			case BuiltInFunction::Tan:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::tan(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("tan: expected a number as argument.");
				}
			case BuiltInFunction::Random:		return CatValue(static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX));
			case BuiltInFunction::RandomRange:
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
			case BuiltInFunction::Round:
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
			case BuiltInFunction::StringRound:
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
			case BuiltInFunction::Abs:
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
			case BuiltInFunction::Cap:
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
			case BuiltInFunction::Min:
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
			case BuiltInFunction::Max:
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
			case BuiltInFunction::Log:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::log10(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("log: expected a number as argument.");
				}
			case BuiltInFunction::Sqrt:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::sqrt(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("sqrt: expected a number as argument.");
				}
			case BuiltInFunction::Pow:
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
			case BuiltInFunction::Ceil:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::ceil(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("ceil: expected a number as argument.");
				}
			case BuiltInFunction::Floor:
				if (isScalar(argumentValues[0].getValueType()))
				{
					return CatValue(std::floor(argumentValues[0].toFloatValue()));
				}
				else
				{
					return CatError("floor: expected a number as argument.");
				}
			case BuiltInFunction::FindInString:
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
			case BuiltInFunction::ReplaceInString:
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
			case BuiltInFunction::StringLength:
				if (isBasicType(argumentValues[0].getValueType()))
				{
					return CatValue((int)argumentValues[0].toStringValue().size());
				}
				else
				{
					return CatError("stringLength: invalid argument.");
				}
			case BuiltInFunction::SubString:
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
			case BuiltInFunction::Select:
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
			case BuiltInFunction::Count:
			case BuiltInFunction::Invalid:
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
	if (function >= BuiltInFunction::Count)
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
			case BuiltInFunction::ToInt:			return CatGenericType(CatType::Int);
			case BuiltInFunction::ToFloat:			return CatGenericType(CatType::Float);
			case BuiltInFunction::ToBool:			return CatGenericType(CatType::Bool);
			case BuiltInFunction::ToString:			return CatGenericType(CatType::String);
			case BuiltInFunction::ToPrettyString:	return CatGenericType(CatType::String);
			case BuiltInFunction::ToFixedLengthString:
				if (argumentTypes[0].isIntType() && argumentTypes[1].isIntType())
				{
					return CatGenericType(CatType::String);
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected an int."));
				}
			case BuiltInFunction::Sin:
			case BuiltInFunction::Cos:
			case BuiltInFunction::Tan:
				if (argumentTypes[0].isScalarType())
				{
					return CatGenericType(CatType::Float);
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected a number as argument."));
				}
			case BuiltInFunction::Random:		return CatGenericType(CatType::Float);
			case BuiltInFunction::RandomRange:
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
			case BuiltInFunction::Round:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					return CatGenericType(CatType::Float);
				}
				else
				{
					return CatGenericType("round: can only round floating point numbers to integer number of decimals.");
				}
			case BuiltInFunction::StringRound:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					return CatGenericType(CatType::String);
				}
				else
				{
					return CatGenericType("stringRound: can only round floating point numbers to integer number of decimals.");
				}
			case BuiltInFunction::Abs:
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
			case BuiltInFunction::Cap:
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
			case BuiltInFunction::Min:
			case BuiltInFunction::Max:
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
			case BuiltInFunction::Log:
			case BuiltInFunction::Sqrt:
			case BuiltInFunction::Ceil:
			case BuiltInFunction::Floor:
				if (argumentTypes[0].isScalarType())
				{
					return CatGenericType(CatType::Float);
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected a number as argument."));
				}
			case BuiltInFunction::Pow:
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
			case BuiltInFunction::FindInString:
				if (argumentTypes[0].isBasicType()
					&& argumentTypes[1].isBasicType())
				{
					return CatGenericType(CatType::Int);
				}
				else
				{
					return CatGenericType("findInString: invalid argument.");
				}
			case BuiltInFunction::ReplaceInString:
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
			case BuiltInFunction::StringLength:
				if (argumentTypes[0].isBasicType())
				{
					return CatGenericType(CatType::Int);
				}
				else
				{
					return CatGenericType("stringLength: invalid argument.");
				}
			case BuiltInFunction::SubString:
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
			case BuiltInFunction::Select:
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
			case BuiltInFunction::Count:
			case BuiltInFunction::Invalid:
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
			case BuiltInFunction::ToInt:
			case BuiltInFunction::StringLength:
			case BuiltInFunction::Round:
			case BuiltInFunction::FindInString:
				return CatType::Int;
			case BuiltInFunction::ToBool:
				return CatType::Bool;
			case BuiltInFunction::RandomRange:
			case BuiltInFunction::Abs:
			case BuiltInFunction::Cap:
			case BuiltInFunction::Min:
			case BuiltInFunction::Max:
				return arguments->arguments[0]->getType();
			case BuiltInFunction::Select:
				return arguments->arguments[1]->getType();
			case BuiltInFunction::ToFloat:
			case BuiltInFunction::Sin:
			case BuiltInFunction::Cos:
			case BuiltInFunction::Tan:
			case BuiltInFunction::Random:
			case BuiltInFunction::Log:
			case BuiltInFunction::Sqrt:
			case BuiltInFunction::Ceil:
			case BuiltInFunction::Floor:
				return CatType::Float;
			case BuiltInFunction::SubString:
			case BuiltInFunction::ToString:
			case BuiltInFunction::ToPrettyString:
			case BuiltInFunction::ToFixedLengthString:
			case BuiltInFunction::StringRound:
			case BuiltInFunction::ReplaceInString:
				return CatType::String;
			case BuiltInFunction::Pow:
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
	else if (function == BuiltInFunction::Select
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


bool CatFunctionCall::isBuiltInFunction(const char* functionName, int numArguments)
{
	return toFunction(functionName, numArguments) != BuiltInFunction::Invalid;
}


const std::vector<std::string>& CatFunctionCall::getAllBuiltInFunctions()
{
	return functionTable;
}


bool CatFunctionCall::isDeterministic() const
{
	return function != BuiltInFunction::Random 
		   && function != BuiltInFunction::RandomRange;
}


bool CatFunctionCall::checkArgumentCount(std::size_t count) const
{
	switch (function)
	{
		default:
		case BuiltInFunction::Random:
			return count == 0;
		case BuiltInFunction::ToInt:
		case BuiltInFunction::ToString:
		case BuiltInFunction::ToPrettyString:
		case BuiltInFunction::StringLength:
		case BuiltInFunction::ToBool:
		case BuiltInFunction::Abs:
		case BuiltInFunction::ToFloat:
		case BuiltInFunction::Sin:
		case BuiltInFunction::Cos:
		case BuiltInFunction::Tan:
		case BuiltInFunction::Log:
		case BuiltInFunction::Sqrt:
		case BuiltInFunction::Ceil:
		case BuiltInFunction::Floor:
			return count == 1;
		case BuiltInFunction::Round:
		case BuiltInFunction::RandomRange:
		case BuiltInFunction::StringRound:
		case BuiltInFunction::Min:
		case BuiltInFunction::Max:
		case BuiltInFunction::Pow:
		case BuiltInFunction::FindInString:
		case BuiltInFunction::ToFixedLengthString:
			return count == 2;
		case BuiltInFunction::Cap:
		case BuiltInFunction::SubString:
		case BuiltInFunction::ReplaceInString:
		case BuiltInFunction::Select:
			return count == 3;
	}
}


CatFunctionCall::BuiltInFunction CatFunctionCall::toFunction(const char* functionName, int numArguments)
{
	for (unsigned int i = 0; i < (unsigned int)BuiltInFunction::Count; i++)
	{
		if (Tools::equalsWhileIgnoringCase(functionTable[i], functionName))
		{
			BuiltInFunction functionType = (BuiltInFunction)i;
			if (functionType == BuiltInFunction::Random)
			{
				if (numArguments == 2)
				{
					return BuiltInFunction::RandomRange;
				}
			}
			return functionType;
		}
	}
	return BuiltInFunction::Invalid;
}


std::vector<std::string> CatFunctionCall::functionTable = 	
{
	 "toInt",				//BuiltInFunction::ToInt
	 "toFloat",				//BuiltInFunction::ToFloat
	 "toBool",				//BuiltInFunction::ToBool
	 "toString",			//BuiltInFunction::ToString
	 "toPrettyString",		//BuiltInFunction::ToPrettyString
	 "toFixedLengthString",	//BuiltInFunction::ToFixedLengthString
	 "sin",					//BuiltInFunction::Sin
	 "cos",					//BuiltInFunction::Cos
	 "tan",					//BuiltInFunction::Tan
	 "rand",				//BuiltInFunction::Random
	 "rand",				//BuiltInFunction::RandomRange
	 "round",				//BuiltInFunction::Round
	 "stringRound",			//BuiltInFunction::StringRound
	 "abs",					//BuiltInFunction::Abs
	 "cap",					//BuiltInFunction::Cap
	 "min",					//BuiltInFunction::Min 
	 "max",					//BuiltInFunction::Max
	 "log",					//BuiltInFunction::Log
	 "sqrt",				//BuiltInFunction::Sqrt
	 "pow",					//BuiltInFunction::Pow
	 "ceil",				//BuiltInFunction::Ceil
	 "floor",				//BuiltInFunction::Floor
	 "findInString",		//BuiltInFunction::FindInString,
	 "replaceInString",		//BuiltInFunction::ReplaceInString,
	 "stringLength",		//BuiltInFunction::StringLength
	 "subString",			//BuiltInFunction::SubString
	 "select"				//BuiltInFunction::Select
};