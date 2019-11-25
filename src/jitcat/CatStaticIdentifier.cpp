/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatStaticIdentifier.h"
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


CatStaticIdentifier::CatStaticIdentifier(CatStaticScope* baseScope, const Tokenizer::Lexeme& identifierLexeme, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	baseScope(baseScope),
	identifier(identifierLexeme),
	identifierLexeme(identifierLexeme),
	type(CatGenericType::unknownType),
	assignableType(CatGenericType::unknownType),
	staticMemberInfo(nullptr)
{
}


CatStaticIdentifier::CatStaticIdentifier(const CatStaticIdentifier& other):
	CatAssignableExpression(other),
	baseScope(other.baseScope != nullptr ? static_cast<CatStaticScope*>(other.baseScope->copy()) : nullptr),
	identifier(other.identifier),
	identifierLexeme(other.identifierLexeme),
	type(other.type),
	assignableType(other.assignableType),
	staticMemberInfo(nullptr)
{
}


CatASTNode* CatStaticIdentifier::copy() const
{
	return new CatStaticIdentifier(*this);
}


const CatGenericType& CatStaticIdentifier::getType() const
{
	return type;
}


const CatGenericType& CatStaticIdentifier::getAssignableType() const
{
	return assignableType;
}


bool jitcat::AST::CatStaticIdentifier::isAssignable() const
{
	return !type.isConst();
}


void CatStaticIdentifier::print() const
{
	baseScope->print();
	Tools::CatLog::log("::", identifier);
}


bool CatStaticIdentifier::isConst() const
{
	return type.isConst();
}


CatTypedExpression* CatStaticIdentifier::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	return this;
}


CatASTNodeType CatStaticIdentifier::getNodeType() const
{
	return CatASTNodeType::StaticIdentifier;
}


std::any CatStaticIdentifier::execute(CatRuntimeContext* runtimeContext)
{
	return staticMemberInfo->getMemberReference();
}


std::any CatStaticIdentifier::executeAssignable(CatRuntimeContext* runtimeContext)
{
	return staticMemberInfo->getAssignableMemberReference();
}


bool CatStaticIdentifier::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
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
		if (type.isPointerType() && type.getOwnershipSemantics() == TypeOwnershipSemantics::Value)
		{
			type.setOwnershipSemantics(TypeOwnershipSemantics::Weak);
		}
		assignableType = type.toPointer(TypeOwnershipSemantics::Weak, type.isWritable(), false);
	}
	else
	{
		errorManager->compiledWithError(std::string("Static member not found: ") + identifier, errorContext, compiletimeContext->getContextName(), identifierLexeme);
		return false;
	}
	return true;
}