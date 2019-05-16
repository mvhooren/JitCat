/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeRoot.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/TypeInfo.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatMemberFunctionCall::CatMemberFunctionCall(const std::string& name, const Tokenizer::Lexeme& nameLexeme, CatTypedExpression* base, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	functionName(name),
	nameLexeme(nameLexeme),
	arguments(arguments),
	base(base),
	memberFunctionInfo(nullptr),
	returnType(CatGenericType::errorType)
{
}


void CatMemberFunctionCall::print() const
{
	if (base != nullptr)\
	{
		base->print();
		CatLog::log(".");
	}
	CatLog::log(functionName);
	arguments->print();
}


CatASTNodeType CatMemberFunctionCall::getNodeType()
{
	return CatASTNodeType::MemberFunctionCall;
}


std::any CatMemberFunctionCall::execute(CatRuntimeContext* runtimeContext)
{
	std::any baseValue = base->execute(runtimeContext);
	return executeWithBase(runtimeContext, baseValue);
}


std::any jitcat::AST::CatMemberFunctionCall::executeWithBase(CatRuntimeContext* runtimeContext, std::any baseValue)
{
	if (memberFunctionInfo != nullptr && runtimeContext != nullptr)
	{
		std::vector<std::any> argumentValues;
		for (std::unique_ptr<CatTypedExpression>& argument : arguments->arguments)
		{
			argumentValues.push_back(argument->execute(runtimeContext));
		}
		return memberFunctionInfo->call(runtimeContext, baseValue, argumentValues);
	}
	assert(false);
	return std::any();
}


bool CatMemberFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	returnType = CatGenericType::errorType;
	if (base == nullptr)
	{
		//function call without a base. Check Scopes.
		CatScopeID scopeId = InvalidScopeID;
		MemberFunctionInfo* memberFunctionInfo = compiletimeContext->findFunction(Tools::toLowerCase(functionName), scopeId);
		if (memberFunctionInfo != nullptr && scopeId != InvalidScopeID)
		{
			base.reset(new CatScopeRoot(scopeId, getLexeme()));
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Function not found: ", functionName, "."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	if (base->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		//Try to collapse the base into a literal. (in case of a static scope, for example)
		ASTHelper::updatePointerIfChanged(base, base->constCollapse(compiletimeContext));

		CatGenericType baseType = base->getType();
		if (!baseType.isObjectType())
		{
			errorManager->compiledWithError(Tools::append("Expression to the left of '.' is not an object."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		memberFunctionInfo = baseType.getObjectType()->getMemberFunctionInfo(Tools::toLowerCase(functionName));
		if (memberFunctionInfo != nullptr)
		{
			std::size_t numArgumentsSupplied = arguments->arguments.size();
			if (numArgumentsSupplied != memberFunctionInfo->getNumberOfArguments())
			{
				errorManager->compiledWithError(Tools::append("Invalid number of arguments for function: ", functionName, " expected ", memberFunctionInfo->getNumberOfArguments(), " arguments."), errorContext, compiletimeContext->getContextName(), getLexeme());
				return false;
			}
			for (unsigned int i = 0; i < numArgumentsSupplied; i++)
			{
				if (arguments->arguments[i]->typeCheck(compiletimeContext, errorManager, errorContext))
				{
					if (!(memberFunctionInfo->getArgumentType(i) == arguments->arguments[i]->getType()))
					{
						errorManager->compiledWithError(Tools::append("Invalid argument for function: ", functionName, " argument nr: ", i, " expected: ", memberFunctionInfo->getArgumentType(i).toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			returnType = memberFunctionInfo->returnType;
			return true;
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Member function not found: ", functionName), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	return false;
}


CatGenericType CatMemberFunctionCall::getType() const
{
	return returnType;
}


bool CatMemberFunctionCall::isConst() const
{
	return false;
}


CatTypedExpression* CatMemberFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext)
{
	ASTHelper::updatePointerIfChanged(base, base->constCollapse(compileTimeContext));
	for (auto& iter: arguments->arguments)
	{
		ASTHelper::updatePointerIfChanged(iter, iter->constCollapse(compileTimeContext));
	}
	return this;
}


MemberFunctionInfo* CatMemberFunctionCall::getMemberFunctionInfo() const
{
	return memberFunctionInfo;
}


CatTypedExpression* CatMemberFunctionCall::getBase() const
{
	return base.get();
}

CatArgumentList* CatMemberFunctionCall::getArguments() const
{
	return arguments.get();
}

const std::string& jitcat::AST::CatMemberFunctionCall::getFunctionName() const
{
	return functionName;
}


void jitcat::AST::CatMemberFunctionCall::setFunctionName(const std::string& name)
{
	functionName = name;
}


void jitcat::AST::CatMemberFunctionCall::setBase(std::unique_ptr<CatTypedExpression> newBase)
{
	base.reset(newBase.release());
}


const Tokenizer::Lexeme& jitcat::AST::CatMemberFunctionCall::getNameLexeme() const
{
	return nameLexeme;
}
