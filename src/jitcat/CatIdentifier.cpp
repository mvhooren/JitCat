/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatIdentifier.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatIdentifier::CatIdentifier(const std::string& name, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	name(name),
	memberInfo(nullptr),
	scopeId(InvalidScopeID),
	type(CatGenericType::errorType)
{
}


CatGenericType CatIdentifier::getType() const
{
	return type;
}


void CatIdentifier::print() const
{
	CatLog::log(name.c_str());
}


bool CatIdentifier::isConst() const
{
	if (memberInfo != nullptr)
	{
		return memberInfo->catType.isConst();
	}
	else
	{
		return true;
	}
}


CatTypedExpression* CatIdentifier::constCollapse(CatRuntimeContext* compileTimeContext_)
{
	return this;
}


CatASTNodeType CatIdentifier::getNodeType()
{
	return CatASTNodeType::Identifier;
}


std::any CatIdentifier::execute(CatRuntimeContext* runtimeContext)
{
	Reflectable* rootObject = runtimeContext->getScopeObject(scopeId);
	return memberInfo->getMemberReference(rootObject);
}


std::any CatIdentifier::executeAssignable(CatRuntimeContext* runtimeContext, AssignableType& assignableType)
{
	Reflectable* rootObject = runtimeContext->getScopeObject(scopeId);
	return memberInfo->getAssignableMemberReference(rootObject, assignableType);
}


bool CatIdentifier::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	std::string lowerName = Tools::toLowerCase(name);
	memberInfo = nullptr;
	type = CatGenericType::errorType;
	if (compiletimeContext != nullptr)
	{
		memberInfo = compiletimeContext->findVariable(lowerName, scopeId);
	}
	if (memberInfo != nullptr)
	{
		type = memberInfo->catType;
	}
	if (type.isValidType())
	{
		return true;
	}
	else
	{
		errorManager->compiledWithError(std::string("Variable not found: ") + name, errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
}


CatScopeID CatIdentifier::getScopeId() const
{
	return scopeId;
}


const TypeMemberInfo* CatIdentifier::getMemberInfo() const
{
	return memberInfo;
}


const std::string& jitcat::AST::CatIdentifier::getName() const
{
	return name;
}
