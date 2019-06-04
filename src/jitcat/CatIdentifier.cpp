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
	type(CatGenericType::unknownType),
	assignableType(CatGenericType::unknownType)
{
}


jitcat::AST::CatIdentifier::CatIdentifier(const CatIdentifier& other):
	CatAssignableExpression(other),
	name(other.name),
	memberInfo(nullptr),
	scopeId(InvalidScopeID),
	type(CatGenericType::unknownType),
	assignableType(CatGenericType::unknownType)
{
}


CatASTNode* jitcat::AST::CatIdentifier::copy() const
{
	return new CatIdentifier(*this);
}


const CatGenericType& CatIdentifier::getType() const
{
	return type;
}


const CatGenericType& jitcat::AST::CatIdentifier::getAssignableType() const
{
	return assignableType;
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


CatASTNodeType CatIdentifier::getNodeType() const
{
	return CatASTNodeType::Identifier;
}


std::any CatIdentifier::execute(CatRuntimeContext* runtimeContext)
{
	Reflectable* rootObject = runtimeContext->getScopeObject(scopeId);
	return memberInfo->getMemberReference(rootObject);
}


std::any CatIdentifier::executeAssignable(CatRuntimeContext* runtimeContext)
{
	Reflectable* rootObject = runtimeContext->getScopeObject(scopeId);
	return memberInfo->getAssignableMemberReference(rootObject);
}


bool CatIdentifier::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	std::string lowerName = Tools::toLowerCase(name);
	memberInfo = nullptr;
	type = CatGenericType::unknownType;
	assignableType = CatGenericType::unknownType;
	if (compiletimeContext != nullptr)
	{
		memberInfo = compiletimeContext->findVariable(lowerName, scopeId);
	}
	if (memberInfo != nullptr)
	{
		type = memberInfo->catType;
		assignableType = type.toPointer(TypeOwnershipSemantics::Weak, type.isWritable(), false);
	}
	if (type.isValidType() && assignableType.isValidType())
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
