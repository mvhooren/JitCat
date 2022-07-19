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
#include "jitcat/CatStaticScope.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/StaticMemberFunctionInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatMemberFunctionCall::CatMemberFunctionCall(const std::string& name, const Tokenizer::Lexeme& nameLexeme, CatTypedExpression* base, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	memberFunctionInfo(nullptr),
	functionName(name),
	lowerCaseFunctionName(Tools::toLowerCase(name)),
	nameLexeme(nameLexeme),
	base(base),
	arguments(arguments),
	returnType(CatGenericType::unknownType)
{
	assert(arguments != nullptr);
}


CatMemberFunctionCall::CatMemberFunctionCall(const CatMemberFunctionCall& other):
	CatAssignableExpression(other),
	memberFunctionInfo(nullptr),
	functionName(other.functionName),
	lowerCaseFunctionName(other.lowerCaseFunctionName),
	nameLexeme(other.nameLexeme),
	base(other.base != nullptr ? static_cast<CatTypedExpression*>(other.base->copy()) : nullptr),
	arguments(static_cast<CatArgumentList*>(other.arguments->copy())),
	returnType(CatGenericType::unknownType)
{
}


CatASTNode* CatMemberFunctionCall::copy() const
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


std::any CatMemberFunctionCall::executeWithBase(CatRuntimeContext* runtimeContext, std::any baseValue)
{
	if (memberFunctionInfo != nullptr && runtimeContext != nullptr)
	{
		bool wasReturning = runtimeContext->getIsReturning();
		runtimeContext->setReturning(false);
		std::vector<std::any> argumentValues;
		argumentValues.reserve(arguments->getNumArguments());
		arguments->executeAllArguments(argumentValues, memberFunctionInfo->getArgumentTypes(), runtimeContext);
		bool isNonNull = arguments->checkArgumentsForNull(argumentsToCheckForNull, argumentValues);
		if (isNonNull)
		{
			std::any value = memberFunctionInfo->call(runtimeContext, baseValue, argumentValues);
			runtimeContext->setReturning(wasReturning);
			return value;
		}
		else
		{
			runtimeContext->setReturning(wasReturning);
			return returnType.createDefault();
		}
	}
	assert(false);
	return std::any();
}


bool CatMemberFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!arguments->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	
	returnType = CatGenericType::unknownType;
	if (base == nullptr)
	{
		//function call without a base. Check Scopes.
		CatScopeID scopeId = InvalidScopeID;
		MemberFunctionInfo* memberFunctionInfo = compiletimeContext->findMemberFunction(this, scopeId);
		if (memberFunctionInfo != nullptr && scopeId != InvalidScopeID)
		{
			base = std::make_unique<CatScopeRoot>(scopeId, getLexeme());
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Function not found: ", functionName, "."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	if (base->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		CatGenericType expectedBaseType = base->getType().removeIndirection().toPointer();
		IndirectionConversionMode conversionMode = IndirectionConversionMode::None;
		bool indirectionConversionSuccess = ASTHelper::doIndirectionConversion(base, expectedBaseType, true, conversionMode);
		//To silence unused variable warning in release builds.
		(void)indirectionConversionSuccess;
		assert(indirectionConversionSuccess);
		
		//Try to collapse the base into a literal. (in case of a static scope, for example)
		ASTHelper::updatePointerIfChanged(base, base->constCollapse(compiletimeContext, errorManager, errorContext));

		CatGenericType baseType = base->getType();
		if (baseType.isPointerToReflectableObjectType()
			|| baseType.isReflectableHandleType())
		{
			memberFunctionInfo = ASTHelper::memberFunctionSearch(functionName, arguments->getArgumentTypes(), baseType.getPointeeType()->getObjectType(), errorManager, compiletimeContext, errorContext, getLexeme());
			if (memberFunctionInfo == nullptr)
			{
				return false;
			}
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Expression to the left of '.' is not an object."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}

		if (memberFunctionInfo != nullptr)
		{
			if (returnType.isReflectableObjectType())
			{
				if (!returnType.isConstructible())
				{
					errorManager->compiledWithError(Tools::append("Function return type is not default constructible: ", functionName, "."), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
				if (!returnType.isCopyConstructible())
				{
					errorManager->compiledWithError(Tools::append("Function return type is not copy constructible: ", functionName, "."), errorContext, compiletimeContext->getContextName(), getLexeme());
					return false;
				}
			}

			if (!arguments->applyIndirectionConversions(memberFunctionInfo->getArgumentTypes(), functionName, compiletimeContext, errorManager, errorContext))
			{
				return false;
			}

			argumentsToCheckForNull.clear();
			for (unsigned int i = 0; i < arguments->getNumArguments(); ++i)
			{
				if (memberFunctionInfo->getArgumentType(i).isNonNullPointerType() && !arguments->getArgumentType(i).isNonNullPointerType())
				{
					argumentsToCheckForNull.push_back(i);
				}
			}

			returnType = memberFunctionInfo->getReturnType();
			return true;
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


CatStatement* CatMemberFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	ASTHelper::updatePointerIfChanged(base, base->constCollapse(compileTimeContext, errorManager, errorContext));
	arguments->constCollapse(compileTimeContext, errorManager, errorContext);
	return this;
}


bool jitcat::AST::CatMemberFunctionCall::isAssignable() const
{
	return returnType.isAssignableType();
}


const CatGenericType& jitcat::AST::CatMemberFunctionCall::getAssignableType() const
{
	return returnType;
}


std::any jitcat::AST::CatMemberFunctionCall::executeAssignable(CatRuntimeContext* runtimeContext)
{
	return execute(runtimeContext);
}


MemberFunctionInfo* CatMemberFunctionCall::getMemberFunctionInfo() const
{
	return memberFunctionInfo;
}


CatTypedExpression* CatMemberFunctionCall::getBase() const
{
	return base.get();
}


const CatArgumentList* CatMemberFunctionCall::getArguments() const
{
	return arguments.get();
}


const std::string& CatMemberFunctionCall::getFunctionName() const
{
	return functionName;
}


void CatMemberFunctionCall::setFunctionName(const std::string& name)
{
	functionName = name;
	lowerCaseFunctionName = Tools::toLowerCase(name);
}


void CatMemberFunctionCall::setBase(std::unique_ptr<CatTypedExpression> newBase)
{
	base.reset(newBase.release());
}


const Tokenizer::Lexeme& CatMemberFunctionCall::getNameLexeme() const
{
	return nameLexeme;
}


const std::vector<int>& CatMemberFunctionCall::getArgumentsToCheckForNull() const
{
	return argumentsToCheckForNull;
}


const std::string& CatMemberFunctionCall::getLowerCaseFunctionName() const
{
	return lowerCaseFunctionName;
}


int CatMemberFunctionCall::getNumParameters() const
{
	return (int)arguments->getNumArguments();
}


const CatGenericType& CatMemberFunctionCall::getParameterType(int index) const
{
	return arguments->getArgumentType(index);
}
