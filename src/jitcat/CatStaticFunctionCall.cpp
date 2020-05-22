/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#include "jitcat/CatStaticFunctionCall.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/StaticMemberFunctionInfo.h"
#include "jitcat/Tools.h"


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatStaticFunctionCall::CatStaticFunctionCall(CatStaticScope* parentScope, const std::string& name, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme):
	CatTypedExpression(lexeme),
	staticFunctionInfo(nullptr),
	parentScope(parentScope),
	name(name),
	lowerCaseName(Tools::toLowerCase(name)),
	nameLexeme(nameLexeme),
	arguments(arguments),
	returnType(CatGenericType::unknownType)
{
}


CatStaticFunctionCall::CatStaticFunctionCall(const CatStaticFunctionCall& other):
	CatTypedExpression(other),
	staticFunctionInfo(nullptr),
	parentScope(static_cast<CatStaticScope*>(other.parentScope->copy())),
	name(other.name),
	lowerCaseName(other.lowerCaseName),
	nameLexeme(other.nameLexeme),
	arguments(static_cast<CatArgumentList*>(other.arguments->copy())),
	returnType(CatGenericType::unknownType)
{
}


const CatArgumentList* jitcat::AST::CatStaticFunctionCall::getArguments() const
{
	return arguments.get();
}


uintptr_t jitcat::AST::CatStaticFunctionCall::getFunctionAddress() const
{
	return staticFunctionInfo->getFunctionAddress();
}


const std::string& jitcat::AST::CatStaticFunctionCall::getFunctionName() const
{
	return name;
}


CatASTNode* CatStaticFunctionCall::copy() const
{
	return new CatStaticFunctionCall(*this);
}


void CatStaticFunctionCall::print() const
{
	parentScope->print();
	CatLog::log("::", name);
	arguments->print();
}


CatASTNodeType CatStaticFunctionCall::getNodeType() const
{
	return CatASTNodeType::StaticFunctionCall;
}


std::any CatStaticFunctionCall::execute(CatRuntimeContext* runtimeContext)
{
	bool wasReturning = runtimeContext->getIsReturning();
	runtimeContext->setReturning(false);
	std::vector<std::any> argumentValues;
	argumentValues.reserve(arguments->getNumArguments());
	arguments->executeAllArguments(argumentValues, staticFunctionInfo->getArgumentTypes(), runtimeContext);
	std::any value = staticFunctionInfo->call(runtimeContext, argumentValues);
	runtimeContext->setReturning(wasReturning);
	return value;
}


bool CatStaticFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!parentScope->typeCheck(compiletimeContext, errorManager, errorContext)
		|| !arguments->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	TypeInfo* parentObjectType =  parentScope->getScopeType();

	staticFunctionInfo = parentObjectType->getStaticMemberFunctionInfo(this);
	if (staticFunctionInfo != nullptr)
	{
		if (returnType.isReflectableObjectType())
		{
			if (!returnType.isConstructible())
			{
				errorManager->compiledWithError(Tools::append("Function return type is not default constructible: ", name, "."), errorContext, compiletimeContext->getContextName(), getLexeme());
				return false;
			}
			if (!returnType.isCopyConstructible())
			{
				errorManager->compiledWithError(Tools::append("Function return type is not copy constructible: ", name, "."), errorContext, compiletimeContext->getContextName(), getLexeme());
				return false;
			}
		}

		std::size_t numArgumentsSupplied = arguments->getNumArguments();
		if (numArgumentsSupplied != staticFunctionInfo->getNumberOfArguments())
		{
			errorManager->compiledWithError(Tools::append("Invalid number of arguments for function: ", name, " expected ", staticFunctionInfo->getNumberOfArguments(), " arguments."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		if (!arguments->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			return false;
		}
		for (unsigned int i = 0; i < numArgumentsSupplied; i++)
		{
			if (!staticFunctionInfo->getArgumentType(i).compare(arguments->getArgumentType(i), false, false))
			{
				errorManager->compiledWithError(Tools::append("Invalid argument for function: ", name, " argument nr: ", i, " expected: ", staticFunctionInfo->getArgumentType(i).toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
				return false;
			}
			else if (!ASTHelper::checkOwnershipSemantics(staticFunctionInfo->getArgumentType(i), arguments->getArgumentType(i), errorManager, compiletimeContext, errorContext, arguments->getArgumentLexeme(i), "pass"))
			{
				return false;
			}
		}
		if (!arguments->applyIndirectionConversions(staticFunctionInfo->getArgumentTypes(), name, compiletimeContext, errorManager, errorContext))
		{
			return false;
		}
		returnType = staticFunctionInfo->getReturnType();
		return true;
	}
	else
	{
		errorManager->compiledWithError(Tools::append("Static function not found: ", name), errorContext, compiletimeContext->getContextName(), nameLexeme);
		return false;
	}
}


const CatGenericType& CatStaticFunctionCall::getType() const
{
	return returnType;
}


bool CatStaticFunctionCall::isConst() const
{
	return false;
}


CatTypedExpression* CatStaticFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	arguments->constCollapse(compileTimeContext, errorManager, errorContext);
	return this;
}


const CatGenericType& jitcat::AST::CatStaticFunctionCall::getFunctionParameterType(std::size_t index) const
{
	assert(staticFunctionInfo != nullptr);
	return staticFunctionInfo->getArgumentType(index);
}

const std::vector<CatGenericType>& jitcat::AST::CatStaticFunctionCall::getExpectedParameterTypes() const
{
	return staticFunctionInfo->getArgumentTypes();
}


const std::string& CatStaticFunctionCall::getLowerCaseFunctionName() const
{
	return lowerCaseName;
}


int CatStaticFunctionCall::getNumParameters() const
{
	return (int)arguments->getNumArguments();
}


const CatGenericType& CatStaticFunctionCall::getParameterType(int index) const
{
	return arguments->getArgumentType(index);
}
