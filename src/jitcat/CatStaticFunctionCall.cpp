/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#include "jitcat/CatStaticFunctionCall.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/StaticMemberFunctionInfo.h"
#include "jitcat/Tools.h"


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatStaticFunctionCall::CatStaticFunctionCall(CatTypeNode* parentType, const std::string& name, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme):
	CatTypedExpression(lexeme),
	staticFunctionInfo(nullptr),
	parentType(parentType),
	name(name),
	nameLexeme(nameLexeme),
	arguments(arguments),
	returnType(CatGenericType::unknownType)
{
}


CatStaticFunctionCall::CatStaticFunctionCall(const CatStaticFunctionCall& other):
	CatTypedExpression(other),
	staticFunctionInfo(nullptr),
	name(other.name),
	nameLexeme(other.nameLexeme),
	parentType(static_cast<CatTypeNode*>(other.parentType->copy())),
	arguments(static_cast<CatArgumentList*>(other.arguments->copy())),
	returnType(CatGenericType::unknownType)
{
}


CatASTNode* CatStaticFunctionCall::copy() const
{
	return new CatStaticFunctionCall(*this);
}


void CatStaticFunctionCall::print() const
{
	parentType->print();
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
	arguments->executeAllArguments(argumentValues, staticFunctionInfo->getArgumentTypes(), runtimeContext);
	std::any value = staticFunctionInfo->call(runtimeContext, argumentValues);
	runtimeContext->setReturning(wasReturning);
	return value;
}


bool CatStaticFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!parentType->typeCheck(compiletimeContext, errorManager, errorContext)
		|| !arguments->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	TypeInfo* parentObjectType =  parentType->getType().getObjectType();
	if (parentObjectType != nullptr)
	{
		staticFunctionInfo = parentObjectType->getStaticMemberFunctionInfo(name);
		if (staticFunctionInfo != nullptr)
		{
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
				if (!staticFunctionInfo->getArgumentType(i).compare(arguments->getArgumentType(i), false))
				{
					errorManager->compiledWithError(Tools::append("Invalid argument for function: ", name, " argument nr: ", i, " expected: ", staticFunctionInfo->getArgumentType(i).toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				else if (!ASTHelper::checkOwnershipSemantics(staticFunctionInfo->getArgumentType(i), arguments->getArgumentType(i), errorManager, compiletimeContext, errorContext, arguments->getArgumentLexeme(i), "pass"))
				{
					return false;
				}
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
	else
	{
		errorManager->compiledWithError(Tools::append("Expression to the left of '::' is not a class type."), errorContext, compiletimeContext->getContextName(), getLexeme());
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


CatTypedExpression* CatStaticFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext)
{
	arguments->constCollapse(compileTimeContext);
	return this;
}
