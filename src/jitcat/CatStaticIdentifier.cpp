/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ASTHelper.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatStaticIdentifier.h"
#include "jitcat/CatStaticMemberAccess.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/StaticConstMemberInfo.h"
#include "jitcat/TypeInfo.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatStaticIdentifier::CatStaticIdentifier(CatStaticScope* baseScope, const Tokenizer::Lexeme& identifierLexeme, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	identifier(identifierLexeme),
	identifierLexeme(identifierLexeme),
	baseScope(baseScope)
{
}


CatStaticIdentifier::CatStaticIdentifier(const CatStaticIdentifier& other):
	CatAssignableExpression(other),
	identifier(other.identifier),
	identifierLexeme(other.identifierLexeme),
	baseScope(other.baseScope != nullptr ? static_cast<CatStaticScope*>(other.baseScope->copy()) : nullptr)
{
}


CatASTNode* CatStaticIdentifier::copy() const
{
	return new CatStaticIdentifier(*this);
}


const CatGenericType& CatStaticIdentifier::getType() const
{
	return disambiguatedIdentifier->getType();
}


const CatGenericType& CatStaticIdentifier::getAssignableType() const
{
	if (disambiguatedIdentifier->isAssignable())
	{
		return static_cast<CatAssignableExpression*>(disambiguatedIdentifier.get())->getAssignableType();
	}
	else
	{
		return CatGenericType::unknownType;
	}
}


bool jitcat::AST::CatStaticIdentifier::isAssignable() const
{
	return disambiguatedIdentifier->isAssignable();
}


void CatStaticIdentifier::print() const
{
	baseScope->print();
	Tools::CatLog::log("::", identifier);
}


bool CatStaticIdentifier::isConst() const
{
	return disambiguatedIdentifier->isConst();
}


CatTypedExpression* CatStaticIdentifier::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	ASTHelper::updatePointerIfChanged(disambiguatedIdentifier, disambiguatedIdentifier->constCollapse(compileTimeContext, errorManager, errorContext));
	return disambiguatedIdentifier.release();
}


CatASTNodeType CatStaticIdentifier::getNodeType() const
{
	return CatASTNodeType::StaticIdentifier;
}


std::any CatStaticIdentifier::execute(CatRuntimeContext* runtimeContext)
{
	return disambiguatedIdentifier->execute(runtimeContext);
}


std::any CatStaticIdentifier::executeAssignable(CatRuntimeContext* runtimeContext)
{
	return static_cast<CatAssignableExpression*>(disambiguatedIdentifier.get())->executeAssignable(runtimeContext);
}


bool CatStaticIdentifier::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (disambiguatedIdentifier != nullptr)
	{
		return disambiguatedIdentifier->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	std::string lowerName = Tools::toLowerCase(identifier);
	assert(baseScope != nullptr);
	if (!baseScope->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	TypeInfo* typeInfo = baseScope->getScopeType();

	if (typeInfo->getStaticMemberInfo(lowerName) != nullptr)
	{
		disambiguatedIdentifier = std::make_unique<CatStaticMemberAccess>(baseScope.release(), identifierLexeme, lexeme);
	}
	else if (StaticConstMemberInfo* constMemberInfo = typeInfo->getStaticConstMemberInfo(lowerName); constMemberInfo != nullptr)
	{
		disambiguatedIdentifier = std::make_unique<CatLiteral>(constMemberInfo->getValue(), constMemberInfo->getType(), lexeme);
	}
	if (disambiguatedIdentifier != nullptr)
	{
		return disambiguatedIdentifier->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	else	
	{
		errorManager->compiledWithError(std::string("Static member not found: ") + identifier, errorContext, compiletimeContext->getContextName(), identifierLexeme);
		return false;
	}
	return true;
}