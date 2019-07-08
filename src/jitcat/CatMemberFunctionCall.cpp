/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeRoot.h"
#include "jitcat/ContainerManipulator.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"
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
	returnType(CatGenericType::unknownType)
{
}


jitcat::AST::CatMemberFunctionCall::CatMemberFunctionCall(const CatMemberFunctionCall& other):
	CatTypedExpression(other),
	functionName(other.functionName),
	nameLexeme(other.nameLexeme),
	arguments(static_cast<CatArgumentList*>(other.arguments->copy())),
	base(other.base != nullptr ? static_cast<CatTypedExpression*>(other.base->copy()) : nullptr),
	memberFunctionInfo(nullptr),
	returnType(CatGenericType::unknownType)
{
}


CatASTNode* jitcat::AST::CatMemberFunctionCall::copy() const
{
	return new CatMemberFunctionCall(*this);
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


CatASTNodeType CatMemberFunctionCall::getNodeType() const
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
		bool wasReturning = runtimeContext->getIsReturning();
		runtimeContext->setReturning(false);
		std::vector<std::any> argumentValues;
		arguments->executeAllArguments(argumentValues, memberFunctionInfo->argumentTypes, runtimeContext);
		std::any value = memberFunctionInfo->call(runtimeContext, baseValue, argumentValues);
		runtimeContext->setReturning(wasReturning);
		return value;
	}
	assert(false);
	return std::any();
}


bool CatMemberFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	returnType = CatGenericType::unknownType;
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
		if (baseType.isPointerToReflectableObjectType()
			|| baseType.isReflectableHandleType())
		{
			memberFunctionInfo = baseType.getPointeeType()->getObjectType()->getMemberFunctionInfo(Tools::toLowerCase(functionName));
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Expression to the left of '.' is not an object."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}

		
		if (memberFunctionInfo != nullptr)
		{
			std::size_t numArgumentsSupplied = arguments->getNumArguments();
			if (numArgumentsSupplied != memberFunctionInfo->getNumberOfArguments())
			{
				errorManager->compiledWithError(Tools::append("Invalid number of arguments for function: ", functionName, " expected ", memberFunctionInfo->getNumberOfArguments(), " arguments."), errorContext, compiletimeContext->getContextName(), getLexeme());
				return false;
			}
			if (!arguments->typeCheck(compiletimeContext, errorManager, errorContext))
			{
				return false;
			}
			for (unsigned int i = 0; i < numArgumentsSupplied; i++)
			{
				if (!memberFunctionInfo->getArgumentType(i).compare(arguments->getArgumentType(i), false))
				{
					errorManager->compiledWithError(Tools::append("Invalid argument for function: ", functionName, " argument nr: ", i, " expected: ", memberFunctionInfo->getArgumentType(i).toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				else if (!ASTHelper::checkOwnershipSemantics(memberFunctionInfo->getArgumentType(i), arguments->getArgumentType(i), errorManager, compiletimeContext, errorContext, arguments->getArgumentLexeme(i), "pass"))
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


const CatGenericType& CatMemberFunctionCall::getType() const
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
	arguments->constCollapse(compileTimeContext);
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
