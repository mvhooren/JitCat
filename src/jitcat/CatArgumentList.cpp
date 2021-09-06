/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatArgumentList.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


CatArgumentList::CatArgumentList(const Tokenizer::Lexeme& lexeme, const std::vector<CatTypedExpression*>& argumentList): 
	CatASTNode(lexeme)
{
	for (auto& iter : argumentList)
	{
		arguments.emplace_back(iter);
	}
}


CatArgumentList::CatArgumentList(const CatArgumentList& other):
	CatASTNode(other)
{
	for (auto& iter : other.arguments)
	{
		arguments.emplace_back(static_cast<CatTypedExpression*>(iter->copy()));
	}
}


CatASTNode* CatArgumentList::copy() const
{
	return new CatArgumentList(*this);
}


void CatArgumentList::print() const
{
	CatLog::log("(");
	for (unsigned int i = 0; i < arguments.size(); i++)
	{
		if (i != 0)
		{
			CatLog::log(", ");
		}
		arguments[i]->print();
	}
	CatLog::log(")");
}


CatASTNodeType CatArgumentList::getNodeType() const
{
	return CatASTNodeType::ParameterList;
}


bool CatArgumentList::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	argumentTypes.clear();
	bool noErrors = true;
	for (auto& iter : arguments)
	{
		if (iter->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			argumentTypes.push_back(iter->getType());
		}
		else
		{
			argumentTypes.push_back(CatGenericType::unknownType);
			noErrors = false;
		}
	}
	return noErrors;
}


void CatArgumentList::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	for (auto& iter : arguments)
	{
		ASTHelper::updatePointerIfChanged(iter, iter->constCollapse(compileTimeContext, errorManager, errorContext));
	}
}


bool CatArgumentList::getAllArgumentsAreConst() const
{
	for (auto& iter : arguments)
	{
		if (!iter->isConst())
		{
			return false;
		}
	}
	return true;
}


bool CatArgumentList::getArgumentIsConst(std::size_t argumentIndex) const
{
	return arguments[argumentIndex]->isConst();
}


Tokenizer::Lexeme CatArgumentList::getArgumentLexeme(std::size_t argumentIndex) const
{
	return arguments[argumentIndex]->getLexeme();
}


CatTypedExpression* CatArgumentList::releaseArgument(std::size_t argumentIndex)
{
	return arguments[argumentIndex].release();
}

std::unique_ptr<CatTypedExpression>& jitcat::AST::CatArgumentList::getArgumentReference(std::size_t argumentIndex)
{
	return arguments[argumentIndex];
}


const CatTypedExpression* CatArgumentList::getArgument(std::size_t argumentIndex) const
{
	return arguments[argumentIndex].get();
}


void CatArgumentList::addArgument(std::unique_ptr<CatTypedExpression> argument)
{
	arguments.emplace_back(std::move(argument));
}


std::any CatArgumentList::executeArgument(std::size_t argumentIndex, CatRuntimeContext* context)
{
	return arguments[argumentIndex]->execute(context);
}


void CatArgumentList::executeAllArguments(std::vector<std::any>& values, const std::vector<CatGenericType>& expectedTypes, CatRuntimeContext* context)
{
	std::size_t index = 0;
	for (auto& iter : arguments)
	{
		std::any value = ASTHelper::doGetArgument(iter.get(), expectedTypes[index], context);
		values.push_back(value);
		index++;
	}
}


const CatGenericType& CatArgumentList::getArgumentType(std::size_t argumentIndex) const
{
	return argumentTypes[argumentIndex];
}


std::size_t CatArgumentList::getNumArguments() const
{
	return arguments.size();
}


const std::vector<CatGenericType>& CatArgumentList::getArgumentTypes() const
{
	return argumentTypes;
}


bool CatArgumentList::applyIndirectionConversions(const std::vector<CatGenericType>& expectedTypes, const std::string& functionName, CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	assert(expectedTypes.size() == arguments.size());
	for (std::size_t i = 0; i < arguments.size(); ++i)
	{
		IndirectionConversionMode mode = IndirectionConversionMode::None;
		if (!ASTHelper::doIndirectionConversion(arguments[i], expectedTypes[i], true, mode))
		{
			switch (mode)
			{
				case IndirectionConversionMode::ErrorNotCopyConstructible:
				{
					errorManager->compiledWithError(Tools::append("Invalid argument for function: ", functionName, " argument nr: ", i, " is not copy constructible."), errorContext, compileTimeContext->getContextName(), getLexeme());
				} break;
				case IndirectionConversionMode::ErrorTooMuchIndirection:
				{
					errorManager->compiledWithError(Tools::append("Invalid argument for function: ", functionName, " argument nr: ", i, " has too much indirection."), errorContext, compileTimeContext->getContextName(), getLexeme());
				} break;
				case IndirectionConversionMode::ErrorTypeMismatch:
				{
					errorManager->compiledWithError(Tools::append("Invalid argument for function: ", functionName, " argument nr: ", i, " is of an unexpected type."), errorContext, compileTimeContext->getContextName(), getLexeme());
				} break;
				default:
				{
					assert(!isValidConversionMode(mode));
				}
			}
			return false;
		}
		argumentTypes[i] = arguments[i]->getType();
	}
	return true;
}


bool CatArgumentList::checkArgumentsForNull(std::vector<int>& argumentsToCheckForNull, std::vector<std::any>& argumentValues)
{
	for (auto iter : argumentsToCheckForNull)
	{
		const CatGenericType& argumentType = getArgumentType(iter);
		if (argumentType.isPointerToReflectableObjectType())
		{
			if (argumentType.removeIndirection().getTypeCaster()->isNullPtr(argumentValues[iter]))
			{
				return false;
			}
		}
		else if (argumentType.isPointerToPointerType() 
				 && !argumentType.getPointeeType()->getPointeeType()->isPointerType())
		{
			if (argumentType.removeIndirection().getTypeCaster()->isNullPtrPtr(argumentValues[iter]))
			{
				return false;
			}
		}
	}
	return true;
}
