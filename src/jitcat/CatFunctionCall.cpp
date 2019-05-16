/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatFunctionCall.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"
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


CatFunctionCall::CatFunctionCall(const std::string& name, const Tokenizer::Lexeme& nameLexeme, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	name(name),
	nameLexeme(nameLexeme),
	arguments(arguments),
	returnType(CatGenericType::errorType),
	function(toFunction(name.c_str(), (int)(arguments->arguments.size())))
{
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
			if (argumentTypes[0].isFloatType())
			{
				return (float)std::sin(std::any_cast<float>(argumentValues[0]));
			}
			else
			{
				return (float)std::sin((float)std::any_cast<int>(argumentValues[0]));
			}
		case CatBuiltInFunctionType::Cos:
			if (argumentTypes[0].isFloatType())
			{
				return (float)std::cos(std::any_cast<float>(argumentValues[0]));
			}
			else
			{
				return (float)std::cos((float)std::any_cast<int>(argumentValues[0]));
			}
		case CatBuiltInFunctionType::Tan:
			if (argumentTypes[0].isFloatType())
			{
				return (float)std::tan(std::any_cast<float>(argumentValues[0]));
			}
			else
			{
				return (float)std::tan((float)std::any_cast<int>(argumentValues[0]));
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


bool CatFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	function = toFunction(name.c_str(), (int)arguments->arguments.size());
	argumentTypes.clear();
	returnType = CatGenericType::errorType;
	std::size_t numArgumentsSupplied = arguments->arguments.size();
	if (function >= CatBuiltInFunctionType::Count)
	{
		errorManager->compiledWithError(Tools::append("function not found: ", name), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	else if (!checkArgumentCount(numArgumentsSupplied))
	{
		errorManager->compiledWithError(Tools::append("Invalid number of arguments in function: " , name), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	else
	{
		for (unsigned int i = 0; i < numArgumentsSupplied; i++)
		{
			if (arguments->arguments[i]->typeCheck(compiletimeContext, errorManager, errorContext))
			{
				argumentTypes.push_back(arguments->arguments[i]->getType());
			}
			else
			{
				return false;
			}
		}

		switch (function)
		{
			case CatBuiltInFunctionType::ToVoid:			returnType = CatGenericType::voidType;	break;
			case CatBuiltInFunctionType::ToInt:				if (argumentTypes[0].isBasicType()) { returnType = CatGenericType::intType		;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to integer: ", argumentTypes[0].toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}  break;
			case CatBuiltInFunctionType::ToFloat:			if (argumentTypes[0].isBasicType()) { returnType = CatGenericType::floatType	;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to float: ", argumentTypes[0].toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}	break;
			case CatBuiltInFunctionType::ToBool:			if (argumentTypes[0].isBasicType()) { returnType = CatGenericType::boolType		;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to boolean: ", argumentTypes[0].toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}  break;
			case CatBuiltInFunctionType::ToString:			if (argumentTypes[0].isBasicType()) { returnType = CatGenericType::stringType	;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to string: ", argumentTypes[0].toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}	break;
			case CatBuiltInFunctionType::ToPrettyString:	if (argumentTypes[0].isBasicType()) { returnType = CatGenericType::stringType	;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to string: ", argumentTypes[0].toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}	break;
			case CatBuiltInFunctionType::ToFixedLengthString:
				if (argumentTypes[0].isIntType() && argumentTypes[1].isIntType())
				{
					returnType = CatGenericType::stringType;
				}
				else
				{
					errorManager->compiledWithError(Tools::append(name, ": expected an int."), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::Sin:
			case CatBuiltInFunctionType::Cos:
			case CatBuiltInFunctionType::Tan:
				if (argumentTypes[0].isScalarType())
				{
					returnType = CatGenericType::floatType;
				}
				else
				{
					errorManager->compiledWithError(Tools::append(name, ": expected a number as argument."), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::Random:		returnType = CatGenericType::floatType; break;
			case CatBuiltInFunctionType::RandomRange:
			{
				if (argumentTypes[0].isBoolType()
					&& argumentTypes[1].isBoolType())
				{
					returnType = CatGenericType::boolType;
				}
				else if (argumentTypes[0].isIntType()
						 && argumentTypes[1].isIntType())
				{
					returnType = CatGenericType::intType;
				}
				else if (argumentTypes[0].isScalarType()
						&& argumentTypes[1].isScalarType())
				{
					returnType = CatGenericType::floatType;
				}
				else
				{
					errorManager->compiledWithError(Tools::append(name, ": invalid argument types."), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			}
			case CatBuiltInFunctionType::Round:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					returnType = CatGenericType::floatType;
				}
				else
				{
					errorManager->compiledWithError("round: can only round floating point numbers to integer number of decimals.", errorContext, compiletimeContext->getContextName(), getLexeme());
				}
				break;
			case CatBuiltInFunctionType::StringRound:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					returnType = CatGenericType::stringType;
				}
				else
				{
					errorManager->compiledWithError("stringRound: can only round floating point numbers to integer number of decimals.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::Abs:
				if (argumentTypes[0].isFloatType())
				{
					returnType = CatGenericType::floatType;
				}
				else if (argumentTypes[0].isIntType())
				{
					returnType = CatGenericType::intType;
				}
				else
				{
					errorManager->compiledWithError("abs: expected a number as argument.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::Cap:
				if (argumentTypes[1].isScalarType()
					&& argumentTypes[2].isScalarType())
				{
					if (argumentTypes[0].isFloatType())
					{
						returnType = CatGenericType::floatType;
					}
					else if (argumentTypes[0].isIntType())
					{
						returnType = CatGenericType::intType;
					}
					else
					{
						errorManager->compiledWithError("cap: value to be capped must be a number.", errorContext, compiletimeContext->getContextName(), getLexeme());
						return false;
					}
				}
				else
				{
					errorManager->compiledWithError("cap: range must consist of 2 numbers.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::Min:
			case CatBuiltInFunctionType::Max:
				if (argumentTypes[0].isFloatType()
					&& argumentTypes[1].isScalarType())
				{
					returnType = CatGenericType::floatType;
				}
				else if (argumentTypes[0].isIntType()
						 && argumentTypes[1].isScalarType())
				{
					returnType = CatGenericType::intType;
				}
				else
				{
					errorManager->compiledWithError(Tools::append(name, ": expected two numbers as arguments."), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::Log:
			case CatBuiltInFunctionType::Sqrt:
			case CatBuiltInFunctionType::Ceil:
			case CatBuiltInFunctionType::Floor:
				if (argumentTypes[0].isScalarType())
				{
					returnType = CatGenericType::floatType;
				}
				else
				{
					errorManager->compiledWithError(Tools::append(name, ": expected a number as argument."), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::Pow:
				if (argumentTypes[0].isScalarType() && argumentTypes[1].isScalarType())
				{
					returnType = CatGenericType::floatType;
				}
				else
				{
					errorManager->compiledWithError("pow: expected two numbers as arguments.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::FindInString:
				if (argumentTypes[0].isBasicType()
					&& argumentTypes[1].isBasicType())
				{
					returnType = CatGenericType::intType;
				}
				else
				{
					errorManager->compiledWithError("findInString: invalid argument.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::ReplaceInString:
				if (argumentTypes[0].isBasicType()
					&& argumentTypes[1].isBasicType()
					&& argumentTypes[2].isBasicType())
				{
					returnType = CatGenericType::stringType;
				}
				else
				{
					errorManager->compiledWithError("replaceInString: invalid argument.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::StringLength:
				if (argumentTypes[0].isBasicType())
				{
					returnType = CatGenericType::intType;
				}
				else
				{
					errorManager->compiledWithError("stringLength: invalid argument.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::SubString:
				if (argumentTypes[0].isBasicType()
					&& argumentTypes[1].isScalarType()
					&& argumentTypes[2].isScalarType())
				{
					returnType = CatGenericType::stringType;
				}
				else
				{
					errorManager->compiledWithError("subString: invalid argument.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				break;
			case CatBuiltInFunctionType::Select:
			{
				if (argumentTypes[0].isBoolType())
				{
					if (argumentTypes[1] == argumentTypes[2]
						|| (argumentTypes[1].isScalarType() && argumentTypes[2].isScalarType()))
					{
						returnType = argumentTypes[1];
					}
					else
					{
						errorManager->compiledWithError("select: second and third argument must be the same type.", errorContext, compiletimeContext->getContextName(), getLexeme());
						return false;
					}
				}
				else
				{
					errorManager->compiledWithError("select: first argument must resolve to a boolean.", errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
			} break;
			default:
			case CatBuiltInFunctionType::Count:
			case CatBuiltInFunctionType::Invalid:
				errorManager->compiledWithError(std::string("function not found: ") + name, errorContext, compiletimeContext->getContextName(), getLexeme());
				return false;
		}
	}
	return true;
}


CatGenericType CatFunctionCall::getType() const
{
	return returnType;
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
		return new CatLiteral(execute(compileTimeContext), getType(), getLexeme());
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


const Tokenizer::Lexeme& jitcat::AST::CatFunctionCall::getNameLexeme() const
{
	return nameLexeme;
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