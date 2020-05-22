/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatStaticMemberAccess.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/TypeInfo.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatStaticMemberAccess::CatStaticMemberAccess(CatStaticScope* baseScope, const Tokenizer::Lexeme& identifierLexeme, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	identifier(identifierLexeme),
	identifierLexeme(identifierLexeme),
	type(CatGenericType::unknownType),
	assignableType(CatGenericType::unknownType),
	baseScope(baseScope),
	staticMemberInfo(nullptr)
{
}


CatStaticMemberAccess::CatStaticMemberAccess(const CatStaticMemberAccess& other):
	CatAssignableExpression(other),
	identifier(other.identifier),
	identifierLexeme(other.identifierLexeme),
	type(other.type),
	assignableType(other.assignableType),
	baseScope(other.baseScope != nullptr ? static_cast<CatStaticScope*>(other.baseScope->copy()) : nullptr),
	staticMemberInfo(nullptr)
{
}


CatASTNode* CatStaticMemberAccess::copy() const
{
	return new CatStaticMemberAccess(*this);
}


const CatGenericType& CatStaticMemberAccess::getType() const
{
	return type;
}


const CatGenericType& CatStaticMemberAccess::getAssignableType() const
{
	return assignableType;
}


bool jitcat::AST::CatStaticMemberAccess::isAssignable() const
{
	return !type.isConst();
}


void CatStaticMemberAccess::print() const
{
	baseScope->print();
	Tools::CatLog::log("::", identifier);
}


bool CatStaticMemberAccess::isConst() const
{
	return type.isConst();
}


CatTypedExpression* CatStaticMemberAccess::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	return this;
}


CatASTNodeType CatStaticMemberAccess::getNodeType() const
{
	return CatASTNodeType::StaticMemberAccess;
}


std::any CatStaticMemberAccess::execute(CatRuntimeContext* runtimeContext)
{
	return staticMemberInfo->getMemberReference();
}


std::any CatStaticMemberAccess::executeAssignable(CatRuntimeContext* runtimeContext)
{
	return staticMemberInfo->getAssignableMemberReference();
}


bool CatStaticMemberAccess::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	std::string lowerName = Tools::toLowerCase(identifier);
	staticMemberInfo = nullptr;
	
	type = CatGenericType::unknownType;
	assignableType = CatGenericType::unknownType;
	assert(baseScope != nullptr);
	if (!baseScope->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	TypeInfo* typeInfo = baseScope->getScopeType();

	staticMemberInfo = typeInfo->getStaticMemberInfo(lowerName);

	if (staticMemberInfo != nullptr)
	{
		type = staticMemberInfo->catType;
		assignableType = type.toPointer(TypeOwnershipSemantics::Weak, type.isWritable(), false);
	}
	else
	{
		errorManager->compiledWithError(std::string("Static member not found: ") + identifier, errorContext, compiletimeContext->getContextName(), identifierLexeme);
		return false;
	}
	return true;
}


Reflection::StaticMemberInfo* jitcat::AST::CatStaticMemberAccess::getStaticMemberInfo() const
{
	return staticMemberInfo;
}
