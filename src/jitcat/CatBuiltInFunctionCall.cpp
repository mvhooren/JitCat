/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatBuiltInFunctionCall.h"
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


CatBuiltInFunctionCall::CatBuiltInFunctionCall(const std::string& name, const Tokenizer::Lexeme& nameLexeme, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	name(name),
	nameLexeme(nameLexeme),
	arguments(arguments),
	returnType(CatGenericType::unknownType),
	function(toFunction(name.c_str(), (int)(arguments->getNumArguments())))
{
}


jitcat::AST::CatBuiltInFunctionCall::CatBuiltInFunctionCall(const CatBuiltInFunctionCall& other):
	CatTypedExpression(other),
	name(other.name),
	nameLexeme(other.nameLexeme),
	arguments(static_cast<CatArgumentList*>(other.arguments->copy())),
	returnType(CatGenericType::unknownType),
	function(toFunction(name.c_str(), (int)(arguments->getNumArguments())))
{
}


CatASTNode* jitcat::AST::CatBuiltInFunctionCall::copy() const
{
	return new CatBuiltInFunctionCall(*this);
}


void CatBuiltInFunctionCall::print() const
{
	CatLog::log(name);
	arguments->print();
}


CatASTNodeType CatBuiltInFunctionCall::getNodeType() const
{
	return CatASTNodeType::FunctionCall;
}


std::any CatBuiltInFunctionCall::execute(CatRuntimeContext* runtimeContext)
{
	std::size_t numArgumentsSupplied = arguments->getNumArguments();
	//At most 3 arguments, check for errors
	std::any argumentValues[3];
	std::size_t numArgumentsToEvaluate = numArgumentsSupplied;
	if (function == CatBuiltInFunctionType::Select)
	{
		numArgumentsToEvaluate = 1;
	}
	for (std::size_t i = 0; i < numArgumentsToEvaluate && i < 3; i++)
	{
		argumentValues[i] = arguments->executeArgument(i, runtimeContext);
	}
	switch (function)
	{
		case CatBuiltInFunctionType::ToVoid:			return std::any();
		case CatBuiltInFunctionType::ToInt:				return CatGenericType::convertToInt(argumentValues[0], arguments->getArgumentType(0));
		case CatBuiltInFunctionType::ToFloat:			return CatGenericType::convertToFloat(argumentValues[0], arguments->getArgumentType(0));
		case CatBuiltInFunctionType::ToBool:			return CatGenericType::convertToBoolean(argumentValues[0], arguments->getArgumentType(0));
		case CatBuiltInFunctionType::ToString:			return CatGenericType::convertToString(argumentValues[0], arguments->getArgumentType(0));
		case CatBuiltInFunctionType::ToPrettyString:
		{
			if (!arguments->getArgumentType(0).isIntType())
			{
				return CatGenericType::convertToString(argumentValues[0], arguments->getArgumentType(0));
			}
			else
			{
				return std::any(LLVMCatIntrinsics::intToPrettyString(std::any_cast<int>(argumentValues[0])));
			}
		}
		case CatBuiltInFunctionType::ToFixedLengthString:	return LLVMCatIntrinsics::intToFixedLengthString(std::any_cast<int>(argumentValues[0]), std::any_cast<int>(argumentValues[1]));
		case CatBuiltInFunctionType::Sin:
			if (arguments->getArgumentType(0).isFloatType())
			{
				return (float)std::sin(std::any_cast<float>(argumentValues[0]));
			}
			else
			{
				return (float)std::sin((float)std::any_cast<int>(argumentValues[0]));
			}
		case CatBuiltInFunctionType::Cos:
			if (arguments->getArgumentType(0).isFloatType())
			{
				return (float)std::cos(std::any_cast<float>(argumentValues[0]));
			}
			else
			{
				return (float)std::cos((float)std::any_cast<int>(argumentValues[0]));
			}
		case CatBuiltInFunctionType::Tan:
			if (arguments->getArgumentType(0).isFloatType())
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
			if (arguments->getArgumentType(0).isBoolType() && arguments->getArgumentType(1).isBoolType())
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
			else if (arguments->getArgumentType(0).isIntType() && arguments->getArgumentType(1).isIntType())
			{
				int min = std::any_cast<int>(argumentValues[0]);
				int max = std::any_cast<int>(argumentValues[1]);
				if (min > max)
				{
					std::swap(min, max);
				}
				return std::any(min + (std::rand() % (max - min + 1)));
			}
			else if (arguments->getArgumentType(0).isScalarType() && arguments->getArgumentType(1).isScalarType())
			{
				float min = std::any_cast<float>(CatGenericType::convertToFloat(argumentValues[0], arguments->getArgumentType(0)));
				float max = std::any_cast<float>(CatGenericType::convertToFloat(argumentValues[1], arguments->getArgumentType(1)));
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
			double multiplier = std::pow(10.0f, CatGenericType::convertToInt(argumentValues[1], arguments->getArgumentType(1)));
			return std::any((float)(std::floor(std::any_cast<float>(argumentValues[0]) * multiplier + 0.5f) / multiplier));
		}
		case CatBuiltInFunctionType::StringRound:
		{
				std::stringstream ss;
				ss.precision(CatGenericType::convertToInt(argumentValues[1], arguments->getArgumentType(1)));
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
			if (arguments->getArgumentType(0).isFloatType())
			{
				return std::any(std::abs(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return std::any(std::abs(std::any_cast<int>(argumentValues[0])));
			}
		case CatBuiltInFunctionType::Cap:
				if (arguments->getArgumentType(0).isFloatType())
				{
					float capValue = std::any_cast<float>(argumentValues[0]);
					float capMin = CatGenericType::convertToFloat(argumentValues[1], arguments->getArgumentType(1));
					float capMax = CatGenericType::convertToFloat(argumentValues[2], arguments->getArgumentType(2));
					if (capMin > capMax)
					{
						std::swap(capMin, capMax);
					}
					return std::max(capMin, std::min(capMax, capValue));
				}
				else
				{
					int capValue = std::any_cast<int>(argumentValues[0]);
					int capMin = CatGenericType::convertToInt(argumentValues[1], arguments->getArgumentType(1));
					int capMax = CatGenericType::convertToInt(argumentValues[2], arguments->getArgumentType(2));
					if (capMin > capMax)
					{
						std::swap(capMin, capMax);
					}
					return std::max(capMin, std::min(capMax, capValue));
				}
		case CatBuiltInFunctionType::Min:
			if (arguments->getArgumentType(0).isFloatType()
				&& arguments->getArgumentType(1).isScalarType())
			{
				return std::any(std::min(std::any_cast<float>(argumentValues[0]), CatGenericType::convertToFloat(argumentValues[1], arguments->getArgumentType(1))));
			}
			else 
			{
				return std::any(std::min(std::any_cast<int>(argumentValues[0]), CatGenericType::convertToInt(argumentValues[1], arguments->getArgumentType(1))));
			}
		case CatBuiltInFunctionType::Max:
			if (arguments->getArgumentType(0).isFloatType()
				&& arguments->getArgumentType(1).isScalarType())
			{
				return std::any(std::max(std::any_cast<float>(argumentValues[0]), CatGenericType::convertToFloat(argumentValues[1], arguments->getArgumentType(1))));
			}
			else 
			{
				return std::any(std::max(std::any_cast<int>(argumentValues[0]), CatGenericType::convertToInt(argumentValues[1], arguments->getArgumentType(1))));
			}
		case CatBuiltInFunctionType::Log:
			if (arguments->getArgumentType(0).isFloatType())
			{
				return std::any((float)std::log10(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return std::any((float)std::log10((float)std::any_cast<int>(argumentValues[0])));
			}
		case CatBuiltInFunctionType::Sqrt:
			if (arguments->getArgumentType(0).isFloatType())
			{
				return std::any((float)std::sqrt(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return std::any((float)std::sqrt((float)std::any_cast<int>(argumentValues[0])));
			}
		case CatBuiltInFunctionType::Pow:		
			if (arguments->getArgumentType(0).isFloatType())
			{
				return std::any((float)std::pow(std::any_cast<float>(argumentValues[0]), CatGenericType::convertToFloat(argumentValues[1], arguments->getArgumentType(1))));
			}
			else 
			{
				return std::any((float)std::pow((float)std::any_cast<int>(argumentValues[0]), CatGenericType::convertToFloat(argumentValues[1], arguments->getArgumentType(1))));
			}
		case CatBuiltInFunctionType::Ceil:
			if (arguments->getArgumentType(0).isFloatType())
			{
				return std::any((float)std::ceil(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return CatGenericType::convertToFloat(argumentValues[0], arguments->getArgumentType(0));
			}
		case CatBuiltInFunctionType::Floor:
			if (arguments->getArgumentType(0).isFloatType())
			{
				return std::any((float)std::floor(std::any_cast<float>(argumentValues[0])));
			}
			else 
			{
				return CatGenericType::convertToFloat(argumentValues[0], arguments->getArgumentType(0));
			}
		case CatBuiltInFunctionType::FindInString:
		{
			std::string stringValue = CatGenericType::convertToString(argumentValues[0], arguments->getArgumentType(0));
			std::string stringToFindValue = CatGenericType::convertToString(argumentValues[1], arguments->getArgumentType(1));
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
			std::string stringValue = CatGenericType::convertToString(argumentValues[0], arguments->getArgumentType(0));
			std::string stringToFindValue = CatGenericType::convertToString(argumentValues[1], arguments->getArgumentType(1));
			std::string replacementStringValue = CatGenericType::convertToString(argumentValues[2], arguments->getArgumentType(2));
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
		case CatBuiltInFunctionType::StringLength:	return std::any((int)CatGenericType::convertToString(argumentValues[0], arguments->getArgumentType(0)).size());
		case CatBuiltInFunctionType::SubString:
		{
			std::string value = CatGenericType::convertToString(argumentValues[0], arguments->getArgumentType(0));
			int offsetValue = CatGenericType::convertToInt(argumentValues[1], arguments->getArgumentType(1));
			if (value.size() == 0 && offsetValue == 0)
			{
				return std::any(std::string(""));
			}
			else if ((int)value.size() > offsetValue && offsetValue >= 0)
			{
				return std::any(value.substr((unsigned int)offsetValue, CatGenericType::convertToInt(argumentValues[2], arguments->getArgumentType(2))));
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
					return arguments->executeArgument(1, runtimeContext);
				}
				else if (arguments->getArgumentType(2).isScalarType())
				{
					return arguments->getArgumentType(1).convertToType(arguments->executeArgument(2, runtimeContext), arguments->getArgumentType(2));
				}
				else
				{
					return arguments->executeArgument(2, runtimeContext);
				}
		}
		default:
		case CatBuiltInFunctionType::Count:
		case CatBuiltInFunctionType::Invalid:
			assert(false);
			return std::any();
	}
}


bool CatBuiltInFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	function = toFunction(name.c_str(), (int)arguments->getNumArguments());
	returnType = CatGenericType::unknownType;
	std::size_t numArgumentsSupplied = arguments->getNumArguments();
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
		if (!arguments->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			return false;
		}

		switch (function)
		{
			case CatBuiltInFunctionType::ToVoid:			returnType = CatGenericType::voidType;	break;
			case CatBuiltInFunctionType::ToInt:				if (arguments->getArgumentType(0).isBasicType()) { returnType = CatGenericType::intType		;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to integer: ",	arguments->getArgumentType(0).toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}  break;
			case CatBuiltInFunctionType::ToFloat:			if (arguments->getArgumentType(0).isBasicType()) { returnType = CatGenericType::floatType	;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to float: ",	arguments->getArgumentType(0).toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}	break;
			case CatBuiltInFunctionType::ToBool:			if (arguments->getArgumentType(0).isBasicType()) { returnType = CatGenericType::boolType	;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to boolean: ",	arguments->getArgumentType(0).toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}  break;
			case CatBuiltInFunctionType::ToString:			if (arguments->getArgumentType(0).isBasicType()) { returnType = CatGenericType::stringType	;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to string: ",	arguments->getArgumentType(0).toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}	break;
			case CatBuiltInFunctionType::ToPrettyString:	if (arguments->getArgumentType(0).isBasicType()) { returnType = CatGenericType::stringType	;} else {errorManager->compiledWithError(Tools::append("Cannot convert type to string: ",	arguments->getArgumentType(0).toString()), errorContext, compiletimeContext->getContextName(), getLexeme()); return false;}	break;
			case CatBuiltInFunctionType::ToFixedLengthString:
				if (arguments->getArgumentType(0).isIntType() && arguments->getArgumentType(1).isIntType())
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
				if (arguments->getArgumentType(0).isScalarType())
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
				if (arguments->getArgumentType(0).isBoolType()
					&& arguments->getArgumentType(1).isBoolType())
				{
					returnType = CatGenericType::boolType;
				}
				else if (arguments->getArgumentType(0).isIntType()
						 && arguments->getArgumentType(1).isIntType())
				{
					returnType = CatGenericType::intType;
				}
				else if (arguments->getArgumentType(0).isScalarType()
						&& arguments->getArgumentType(1).isScalarType())
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
				if (arguments->getArgumentType(0).isFloatType()
					&& arguments->getArgumentType(1).isScalarType())
				{
					returnType = CatGenericType::floatType;
				}
				else
				{
					errorManager->compiledWithError("round: can only round floating point numbers to integer number of decimals.", errorContext, compiletimeContext->getContextName(), getLexeme());
				}
				break;
			case CatBuiltInFunctionType::StringRound:
				if (arguments->getArgumentType(0).isFloatType()
					&& arguments->getArgumentType(1).isScalarType())
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
				if (arguments->getArgumentType(0).isFloatType())
				{
					returnType = CatGenericType::floatType;
				}
				else if (arguments->getArgumentType(0).isIntType())
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
				if (arguments->getArgumentType(1).isScalarType()
					&& arguments->getArgumentType(2).isScalarType())
				{
					if (arguments->getArgumentType(0).isFloatType())
					{
						returnType = CatGenericType::floatType;
					}
					else if (arguments->getArgumentType(0).isIntType())
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
				if (arguments->getArgumentType(0).isFloatType()
					&& arguments->getArgumentType(1).isScalarType())
				{
					returnType = CatGenericType::floatType;
				}
				else if (arguments->getArgumentType(0).isIntType()
						 && arguments->getArgumentType(1).isScalarType())
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
				if (arguments->getArgumentType(0).isScalarType())
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
				if (arguments->getArgumentType(0).isScalarType() && arguments->getArgumentType(1).isScalarType())
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
				if (arguments->getArgumentType(0).isBasicType()
					&& arguments->getArgumentType(1).isBasicType())
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
				if (arguments->getArgumentType(0).isBasicType()
					&& arguments->getArgumentType(1).isBasicType()
					&& arguments->getArgumentType(2).isBasicType())
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
				if (arguments->getArgumentType(0).isBasicType())
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
				if (arguments->getArgumentType(0).isBasicType()
					&& arguments->getArgumentType(1).isScalarType()
					&& arguments->getArgumentType(2).isScalarType())
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
				if (arguments->getArgumentType(0).isBoolType())
				{
					if (arguments->getArgumentType(1) == arguments->getArgumentType(2)
						|| (arguments->getArgumentType(1).isScalarType() && arguments->getArgumentType(2).isScalarType()))
					{
						returnType = arguments->getArgumentType(1);
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


const CatGenericType& CatBuiltInFunctionCall::getType() const
{
	return returnType;
}


bool CatBuiltInFunctionCall::isConst() const
{
	if (isDeterministic())
	{
		return arguments->getAllArgumentsAreConst();
	}
	else
	{
		return false;
	}
}


CatTypedExpression* CatBuiltInFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext)
{
	arguments->constCollapse(compileTimeContext);
	if (isDeterministic() && arguments->getAllArgumentsAreConst())
	{
		return new CatLiteral(execute(compileTimeContext), getType(), getLexeme());
	}
	else if (function == CatBuiltInFunctionType::Select
			 && arguments->getArgumentIsConst(0))
	{
		bool value = std::any_cast<bool>(arguments->executeArgument(0, compileTimeContext));
		if (value)
		{
			return arguments->releaseArgument(1);
		}
		else
		{
			return arguments->releaseArgument(2);
		}
	}
	return this;
}


CatBuiltInFunctionType CatBuiltInFunctionCall::getFunctionType() const
{
	return function;
}


const std::string& CatBuiltInFunctionCall::getFunctionName() const
{
	return name;
}


const Tokenizer::Lexeme& jitcat::AST::CatBuiltInFunctionCall::getNameLexeme() const
{
	return nameLexeme;
}


CatArgumentList* CatBuiltInFunctionCall::getArgumentList() const
{
	return arguments.get();
}


bool CatBuiltInFunctionCall::isBuiltInFunction(const char* functionName, int numArguments)
{
	return toFunction(functionName, numArguments) != CatBuiltInFunctionType::Invalid;
}


const std::vector<std::string>& CatBuiltInFunctionCall::getAllBuiltInFunctions()
{
	return functionTable;
}


bool CatBuiltInFunctionCall::isDeterministic() const
{
	return function != CatBuiltInFunctionType::Random 
		   && function != CatBuiltInFunctionType::RandomRange;
}


bool CatBuiltInFunctionCall::checkArgumentCount(std::size_t count) const
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


CatBuiltInFunctionType CatBuiltInFunctionCall::toFunction(const char* functionName, int numArguments)
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


std::vector<std::string> CatBuiltInFunctionCall::functionTable = 	
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