/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatFunctionCall.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/JitCat.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/Tools.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::LLVM;
using namespace jitcat::Tools;


CatFunctionCall::CatFunctionCall(const std::string& name, CatArgumentList* arguments):
	name(name),
	arguments(arguments)
{
	function = toFunction(name.c_str(), (int)arguments->arguments.size());
	for (auto& iter : arguments->arguments)
	{
		argumentTypes.push_back(iter->getType());
	}
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


std::any CatFunctionCall::execute(CatRuntimeContext* runtimeContext)
{
	std::size_t numArgumentsSupplied = arguments->arguments.size();
	//At most 3 arguments, check for errors
	std::any argumentValues[3];
	std::size_t numArgumentsToEvaluate = numArgumentsSupplied;
	if (function == CatBuiltInFunctionType::Select)
	{
		numArgumentsToEvaluate = 1;
	}
	for (unsigned int i = 0; i < numArgumentsToEvaluate && i < 3; i++)
	{
		argumentValues[i] = arguments->arguments[i]->execute(runtimeContext);
	}
	switch (function)
	{
		case CatBuiltInFunctionType::ToVoid:			return std::any();
		case CatBuiltInFunctionType::ToInt:				return CatGenericType::convertToInt(argumentValues[0], argumentTypes[0]);
		case CatBuiltInFunctionType::ToFloat:			return CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]);
		case CatBuiltInFunctionType::ToBool:			return CatGenericType::convertToBoolean(argumentValues[0], argumentTypes[0]);
		case CatBuiltInFunctionType::ToString:			return CatGenericType::convertToString(argumentValues[0], argumentTypes[0]);
		case CatBuiltInFunctionType::ToPrettyString:
		{
			if (!argumentTypes[0].isIntType())
			{
				return CatGenericType::convertToString(argumentValues[0], argumentTypes[0]);
			}
			else
			{
				return std::any(LLVMCatIntrinsics::intToPrettyString(std::any_cast<int>(argumentValues[0])));
			}
		}
		case CatBuiltInFunctionType::ToFixedLengthString:	return LLVMCatIntrinsics::intToFixedLengthString(std::any_cast<int>(argumentValues[0]), std::any_cast<int>(argumentValues[1]));

		case CatBuiltInFunctionType::Sin:
			return (float)std::sin(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Cos:
			return (float)std::cos(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Tan:
			return (float)std::tan(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Asin:
			return (float)std::asin(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Acos:
			return (float)std::acos(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Atan:
			return (float)std::atan(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));

		case CatBuiltInFunctionType::Sinh:
			return (float)std::sinh(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Cosh:
			return (float)std::cosh(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Tanh:
			return (float)std::tanh(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Asinh:
			return (float)std::asinh(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Acosh:
			return (float)std::acosh(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Atanh:
			return (float)std::atanh(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Atan2:
		{
			float y = CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]);
			float x = CatGenericType::convertToFloat(argumentValues[1], argumentTypes[1]);
			return (float)std::atan2(y, x);
		}
		case CatBuiltInFunctionType::Hypot:
		{
			float x = CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]);
			float y = CatGenericType::convertToFloat(argumentValues[1], argumentTypes[1]);
			return (float)std::hypot(x, y);
		}

		case CatBuiltInFunctionType::Random:		return std::any(static_cast<float> (std::rand()) / static_cast <float> (RAND_MAX));
		case CatBuiltInFunctionType::RandomRange:
		{
			if (argumentTypes[0].isBoolType() && argumentTypes[1].isBoolType())
			{
				if (std::any_cast<bool>(argumentValues[0]) != std::any_cast<bool>(argumentValues[1]))
				{
					return (std::rand() % 2) == 1 ? true : false;
				}
				else
				{
					return argumentValues[0];
				}
			}
			else if (argumentTypes[0].isIntType() && argumentTypes[1].isIntType())
			{
				int min = std::any_cast<int>(argumentValues[0]);
				int max = std::any_cast<int>(argumentValues[1]);
				if (min > max)
				{
					std::swap(min, max);
				}
				return std::any(min + (std::rand() % (max - min + 1)));
			}
			else if (argumentTypes[0].isScalarType() && argumentTypes[1].isScalarType())
			{
				float min = std::any_cast<float>(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
				float max = std::any_cast<float>(CatGenericType::convertToFloat(argumentValues[1], argumentTypes[1]));
				if (min > max)
				{
					std::swap(min, max);
				}
				float random = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
				return std::any(min + random * (max - min));
			}
			else
			{
				assert(false);
				return std::any();
			}
		}
		case CatBuiltInFunctionType::Round:
		{
			double multiplier = std::pow(10.0f, CatGenericType::convertToInt(argumentValues[1], argumentTypes[1]));
			return std::any((float)(std::floor(std::any_cast<float>(argumentValues[0]) * multiplier + 0.5f) / multiplier));
		}
		case CatBuiltInFunctionType::StringRound:
		{
				std::stringstream ss;
				ss.precision(CatGenericType::convertToInt(argumentValues[1], argumentTypes[1]));
				ss.setf(std::ios_base::fixed);
				ss.unsetf(std::ios_base::scientific);
				ss << std::any_cast<float>(argumentValues[0]);
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
				return std::any(result.substr(0, result.length() - discardedCharacters));
		}
		case CatBuiltInFunctionType::Abs:
			if (argumentTypes[0].isFloatType())
			{
				return std::any(std::abs(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return std::any(std::abs(std::any_cast<int>(argumentValues[0])));
			}
		case CatBuiltInFunctionType::Cap:
				if (argumentTypes[0].isFloatType())
				{
					float capValue = std::any_cast<float>(argumentValues[0]);
					float capMin = CatGenericType::convertToFloat(argumentValues[1], argumentTypes[1]);
					float capMax = CatGenericType::convertToFloat(argumentValues[2], argumentTypes[2]);
					if (capMin > capMax)
					{
						std::swap(capMin, capMax);
					}
					return std::max(capMin, std::min(capMax, capValue));
				}
				else
				{
					int capValue = std::any_cast<int>(argumentValues[0]);
					int capMin = CatGenericType::convertToInt(argumentValues[1], argumentTypes[1]);
					int capMax = CatGenericType::convertToInt(argumentValues[2], argumentTypes[2]); 
					if (capMin > capMax)
					{
						std::swap(capMin, capMax);
					}
					return std::max(capMin, std::min(capMax, capValue));
				}
		case CatBuiltInFunctionType::Min:
			if (argumentTypes[0].isFloatType()
				&& argumentTypes[1].isScalarType())
			{
				return std::any(std::min(std::any_cast<float>(argumentValues[0]), CatGenericType::convertToFloat(argumentValues[1], argumentTypes[1])));
			}
			else 
			{
				return std::any(std::min(std::any_cast<int>(argumentValues[0]), CatGenericType::convertToInt(argumentValues[1], argumentTypes[1])));
			}
		case CatBuiltInFunctionType::Max:
			if (argumentTypes[0].isFloatType()
				&& argumentTypes[1].isScalarType())
			{
				return std::any(std::max(std::any_cast<float>(argumentValues[0]), CatGenericType::convertToFloat(argumentValues[1], argumentTypes[1])));
			}
			else 
			{
				return std::any(std::max(std::any_cast<int>(argumentValues[0]), CatGenericType::convertToInt(argumentValues[1], argumentTypes[1])));
			}
		case CatBuiltInFunctionType::Log:
			if (argumentTypes[0].isFloatType())
			{
				return std::any((float)std::log10(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return std::any((float)std::log10((float)std::any_cast<int>(argumentValues[0])));
			}
		case CatBuiltInFunctionType::Ln:
			return (float)std::log(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Exp:
			return (float)std::exp(CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]));
		case CatBuiltInFunctionType::Sqrt:
			if (argumentTypes[0].isFloatType())
			{
				return std::any((float)std::sqrt(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return std::any((float)std::sqrt((float)std::any_cast<int>(argumentValues[0])));
			}
		case CatBuiltInFunctionType::Pow:		
			if (argumentTypes[0].isFloatType())
			{
				return std::any((float)std::pow(std::any_cast<float>(argumentValues[0]), CatGenericType::convertToFloat(argumentValues[1], argumentTypes[1])));
			}
			else 
			{
				return std::any((float)std::pow((float)std::any_cast<int>(argumentValues[0]), CatGenericType::convertToFloat(argumentValues[1], argumentTypes[1])));
			}
		case CatBuiltInFunctionType::Ceil:
			if (argumentTypes[0].isFloatType())
			{
				return std::any((float)std::ceil(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]);
			}
		case CatBuiltInFunctionType::Floor:
			if (argumentTypes[0].isFloatType())
			{
				return std::any((float)std::floor(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return CatGenericType::convertToFloat(argumentValues[0], argumentTypes[0]);
			}
		case CatBuiltInFunctionType::FindInString:
		{
			std::string stringValue = CatGenericType::convertToString(argumentValues[0], argumentTypes[0]);
			std::string stringToFindValue = CatGenericType::convertToString(argumentValues[1], argumentTypes[1]);
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
			return std::any(result);
		}
		case CatBuiltInFunctionType::ReplaceInString:
		{
			std::string stringValue = CatGenericType::convertToString(argumentValues[0], argumentTypes[0]);
			std::string stringToFindValue = CatGenericType::convertToString(argumentValues[1], argumentTypes[1]);
			std::string replacementStringValue = CatGenericType::convertToString(argumentValues[2], argumentTypes[2]);
			if (stringToFindValue != "")
			{
				size_t startPosition = 0;
				while ((startPosition = stringValue.find(stringToFindValue, startPosition)) != std::string::npos)
				{
					stringValue.replace(startPosition, stringToFindValue.length(), replacementStringValue);
					startPosition += replacementStringValue.length(); 
				}
			}
			return std::any(stringValue);
		}
		case CatBuiltInFunctionType::StringLength:	return std::any((int)CatGenericType::convertToString(argumentValues[0], argumentTypes[0]).size());
		case CatBuiltInFunctionType::SubString:
		{
			std::string value = CatGenericType::convertToString(argumentValues[0], argumentTypes[0]);
			int offsetValue = CatGenericType::convertToInt(argumentValues[1], argumentTypes[1]);
			if (value.size() == 0 && offsetValue == 0)
			{
				return std::any(std::string(""));
			}
			else if ((int)value.size() > offsetValue && offsetValue >= 0)
			{
				return std::any(value.substr((unsigned int)offsetValue, CatGenericType::convertToInt(argumentValues[2], argumentTypes[2])));
			}
			else
			{
				return std::any(std::string(""));
			}
		}
		case CatBuiltInFunctionType::Select:
		{
				if (std::any_cast<bool>(argumentValues[0]))
				{
					return arguments->arguments[1]->execute(runtimeContext);
				}
				else if (argumentTypes[2].isScalarType())
				{
					return argumentTypes[1].convertToType(arguments->arguments[2]->execute(runtimeContext), argumentTypes[2]);
				}
				else
				{
					return arguments->arguments[2]->execute(runtimeContext);
				}
		}
		default:
		case CatBuiltInFunctionType::Count:
		case CatBuiltInFunctionType::Invalid:
			return std::any();
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
			case CatBuiltInFunctionType::ToVoid:			return CatGenericType::voidType;
			case CatBuiltInFunctionType::ToInt:				return argumentTypes[0].isBasicType() ? CatGenericType::intType : CatGenericType(Tools::append("Cannot convert type to integer: ", argumentTypes[0].toString()));
			case CatBuiltInFunctionType::ToFloat:			return argumentTypes[0].isBasicType() ? CatGenericType::floatType : CatGenericType(Tools::append("Cannot convert type to float: ", argumentTypes[0].toString()));
			case CatBuiltInFunctionType::ToBool:			return argumentTypes[0].isBasicType() ? CatGenericType::boolType : CatGenericType(Tools::append("Cannot convert type to boolean: ", argumentTypes[0].toString()));
			case CatBuiltInFunctionType::ToString:			return argumentTypes[0].isBasicType() ? CatGenericType::stringType : CatGenericType(Tools::append("Cannot convert type to string: ", argumentTypes[0].toString()));
			case CatBuiltInFunctionType::ToPrettyString:	return argumentTypes[0].isBasicType() ? CatGenericType::stringType : CatGenericType(Tools::append("Cannot convert type to string: ", argumentTypes[0].toString()));
			case CatBuiltInFunctionType::ToFixedLengthString:
				if (argumentTypes[0].isIntType() && argumentTypes[1].isIntType())
				{
					return CatGenericType::stringType;
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected an int."));
				}
			case CatBuiltInFunctionType::Sin:
			case CatBuiltInFunctionType::Cos:
			case CatBuiltInFunctionType::Tan:
			case CatBuiltInFunctionType::Asin:
			case CatBuiltInFunctionType::Acos:
			case CatBuiltInFunctionType::Atan:
			case CatBuiltInFunctionType::Sinh:
			case CatBuiltInFunctionType::Cosh:
			case CatBuiltInFunctionType::Tanh:
			case CatBuiltInFunctionType::Asinh:
			case CatBuiltInFunctionType::Acosh:
			case CatBuiltInFunctionType::Atanh:
				if (argumentTypes[0].isScalarType())
				{
					return CatGenericType::floatType;
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected a number as argument."));
				}
			case CatBuiltInFunctionType::Atan2:
				if (argumentTypes[0].isScalarType() && argumentTypes[1].isScalarType())
				{
					return CatGenericType::floatType;
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected two numbers as arguments."));
				}
			case CatBuiltInFunctionType::Hypot:
				if (argumentTypes[0].isScalarType() && argumentTypes[1].isScalarType())
				{
					return CatGenericType::floatType;
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected two numbers as arguments."));
				}
			case CatBuiltInFunctionType::Random:		return CatGenericType::floatType;
			case CatBuiltInFunctionType::RandomRange:
			{
				if (argumentTypes[0].isBoolType()
					&& argumentTypes[1].isBoolType())
				{
					return CatGenericType::boolType;
				}
				else if (argumentTypes[0].isIntType()
						 && argumentTypes[1].isIntType())
				{
					return CatGenericType::intType;
				}
				else if (argumentTypes[0].isScalarType()
						&& argumentTypes[1].isScalarType())
				{
					return CatGenericType::floatType;
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
					return CatGenericType::floatType;
				}
				else
				{
					return CatGenericType("round: can only round floating point numbers to integer number of decimals.");
				}
			case CatBuiltInFunctionType::StringRound:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					return CatGenericType::stringType;
				}
				else
				{
					return CatGenericType("stringRound: can only round floating point numbers to integer number of decimals.");
				}
			case CatBuiltInFunctionType::Abs:
				if (argumentTypes[0].isFloatType())
				{
					return CatGenericType::floatType;
				}
				else if (argumentTypes[0].isIntType())
				{
					return CatGenericType::intType;
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
						return CatGenericType::floatType;
					}
					else if (argumentTypes[0].isIntType())
					{
						return CatGenericType::intType;
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
					return CatGenericType::floatType;
				}
				else if (argumentTypes[0].isIntType()
						 && argumentTypes[1].isScalarType())
				{
					return CatGenericType::intType;
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected two numbers as arguments."));
				}
			case CatBuiltInFunctionType::Log:
			case CatBuiltInFunctionType::Ln:
			case CatBuiltInFunctionType::Exp:
			case CatBuiltInFunctionType::Sqrt:
			case CatBuiltInFunctionType::Ceil:
			case CatBuiltInFunctionType::Floor:
				if (argumentTypes[0].isScalarType())
				{
					return CatGenericType::floatType;
				}
				else
				{
					return CatGenericType(Tools::append(name, ": expected a number as argument."));
				}
			case CatBuiltInFunctionType::Pow:
				if (argumentTypes[0].isScalarType() && argumentTypes[1].isScalarType())
				{
					return CatGenericType::floatType;
				}
				else
				{
					return CatGenericType("pow: expected two numbers as arguments.");
				}
			case CatBuiltInFunctionType::FindInString:
				if (argumentTypes[0].isBasicType()
					&& argumentTypes[1].isBasicType())
				{
					return CatGenericType::intType;
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
					return CatGenericType::stringType;
				}
				else
				{
					return CatGenericType("replaceInString: invalid argument.");
				}
			case CatBuiltInFunctionType::StringLength:
				if (argumentTypes[0].isBasicType())
				{
					return CatGenericType::intType;
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
					return CatGenericType::stringType;
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
				return CatGenericType::errorType;
			case CatBuiltInFunctionType::ToVoid:
				return CatGenericType::voidType;
			case CatBuiltInFunctionType::ToInt:
			case CatBuiltInFunctionType::StringLength:
			case CatBuiltInFunctionType::FindInString:
				return CatGenericType::intType;
			case CatBuiltInFunctionType::ToBool:
				return CatGenericType::boolType;
			case CatBuiltInFunctionType::RandomRange:
			{
				if (arguments->arguments[0]->getType().isBoolType() && arguments->arguments[1]->getType().isBoolType())
				{
					return CatGenericType::boolType;
				}
				//Intentional fall-through
			}
			case CatBuiltInFunctionType::Min:
			case CatBuiltInFunctionType::Max:
			{
				if (arguments->arguments[0]->getType().isIntType() && arguments->arguments[1]->getType().isIntType())
				{
					return CatGenericType::intType;
				}
				else
				{
					return CatGenericType::floatType;
				}
			}
			case CatBuiltInFunctionType::Abs:
			case CatBuiltInFunctionType::Cap:
				return arguments->arguments[0]->getType();
			case CatBuiltInFunctionType::Select:
				return arguments->arguments[1]->getType();
			case CatBuiltInFunctionType::ToFloat:
			case CatBuiltInFunctionType::Round:
			case CatBuiltInFunctionType::Sin:
			case CatBuiltInFunctionType::Cos:
			case CatBuiltInFunctionType::Tan:
			case CatBuiltInFunctionType::Asin:
			case CatBuiltInFunctionType::Acos:
			case CatBuiltInFunctionType::Atan:
			case CatBuiltInFunctionType::Sinh:
			case CatBuiltInFunctionType::Cosh:
			case CatBuiltInFunctionType::Tanh:
			case CatBuiltInFunctionType::Asinh:
			case CatBuiltInFunctionType::Acosh:
			case CatBuiltInFunctionType::Atanh:
			case CatBuiltInFunctionType::Atan2:
			case CatBuiltInFunctionType::Hypot:
			case CatBuiltInFunctionType::Random:
			case CatBuiltInFunctionType::Log:
			case CatBuiltInFunctionType::Ln:
			case CatBuiltInFunctionType::Exp:
			case CatBuiltInFunctionType::Pow:
			case CatBuiltInFunctionType::Sqrt:
			case CatBuiltInFunctionType::Ceil:
			case CatBuiltInFunctionType::Floor:
				return CatGenericType::floatType;
			case CatBuiltInFunctionType::SubString:
			case CatBuiltInFunctionType::ToString:
			case CatBuiltInFunctionType::ToPrettyString:
			case CatBuiltInFunctionType::ToFixedLengthString:
			case CatBuiltInFunctionType::StringRound:
			case CatBuiltInFunctionType::ReplaceInString:
				return CatGenericType::stringType;
		}
	}
	else
	{
		return CatGenericType("Unknown function");
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
		ASTHelper::updatePointerIfChanged(iter, iter->constCollapse(compileTimeContext));
		if (!iter->isConst())
		{
			allArgumentsAreConst = false;
		}
	}
	if (isDeterministic() && allArgumentsAreConst)
	{
		return new CatLiteral(execute(compileTimeContext), getType());
	}
	else if (function == CatBuiltInFunctionType::Select
			 && arguments->arguments[0]->isConst())
	{
		bool value = std::any_cast<bool>(arguments->arguments[0]->execute(compileTimeContext));
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
		case CatBuiltInFunctionType::ToVoid:
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
		case CatBuiltInFunctionType::Asin:
		case CatBuiltInFunctionType::Acos:
		case CatBuiltInFunctionType::Atan:
		case CatBuiltInFunctionType::Sinh:
		case CatBuiltInFunctionType::Cosh:
		case CatBuiltInFunctionType::Tanh:
		case CatBuiltInFunctionType::Asinh:
		case CatBuiltInFunctionType::Acosh:
		case CatBuiltInFunctionType::Atanh:
		case CatBuiltInFunctionType::Log:
		case CatBuiltInFunctionType::Ln:
		case CatBuiltInFunctionType::Exp:
		case CatBuiltInFunctionType::Sqrt:
		case CatBuiltInFunctionType::Ceil:
		case CatBuiltInFunctionType::Floor:
			return count == 1;
		case CatBuiltInFunctionType::Atan2:
		case CatBuiltInFunctionType::Hypot:
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
	 "toVoid",				//CatBuiltInFunctionType::ToVoid
	 "toInt",				//CatBuiltInFunctionType::ToInt
	 "toFloat",				//CatBuiltInFunctionType::ToFloat
	 "toBool",				//CatBuiltInFunctionType::ToBool
	 "toString",			//CatBuiltInFunctionType::ToString
	 "toPrettyString",		//CatBuiltInFunctionType::ToPrettyString
	 "toFixedLengthString",	//CatBuiltInFunctionType::ToFixedLengthString
	 "sin",					//CatBuiltInFunctionType::Sin
	 "cos",					//CatBuiltInFunctionType::Cos
	 "tan",					//CatBuiltInFunctionType::Tan
	 "asin",				//CatBuiltInFunctionType::Asin
	 "acos",				//CatBuiltInFunctionType::Acos
	 "atan",				//CatBuiltInFunctionType::Atan
	 "sinh",				//CatBuiltInFunctionType::Sinh
	 "cosh",				//CatBuiltInFunctionType::Cosh
	 "tanh",				//CatBuiltInFunctionType::Tanh
	 "asinh",				//CatBuiltInFunctionType::Asinh
	 "acosh",				//CatBuiltInFunctionType::Acosh
	 "atanh",				//CatBuiltInFunctionType::Atanh
	 "atan2",				//CatBuiltInFunctionType::Atan2
	 "hypot",				//CatBuiltInFunctionType::Hypot
	 "rand",				//CatBuiltInFunctionType::Random
	 "rand",				//CatBuiltInFunctionType::RandomRange
	 "round",				//CatBuiltInFunctionType::Round
	 "stringRound",			//CatBuiltInFunctionType::StringRound
	 "abs",					//CatBuiltInFunctionType::Abs
	 "cap",					//CatBuiltInFunctionType::Cap
	 "min",					//CatBuiltInFunctionType::Min 
	 "max",					//CatBuiltInFunctionType::Max
	 "log",					//CatBuiltInFunctionType::Log
	 "ln",					//CatBuiltInFunctionType::Ln
	 "exp",					//CatBuiltInFunctionType::Exp
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