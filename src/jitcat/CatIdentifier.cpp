/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ASTHelper.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberAccess.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeRoot.h"
#include "jitcat/CatStaticMemberAccess.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/StaticConstMemberInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/MemberInfo.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatIdentifier::CatIdentifier(const std::string& name, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	name(name)
{
}


jitcat::AST::CatIdentifier::CatIdentifier(const CatIdentifier& other):
	CatAssignableExpression(other),
	name(other.name)
{
}


CatASTNode* jitcat::AST::CatIdentifier::copy() const
{
	return new CatIdentifier(*this);
}


const CatGenericType& CatIdentifier::getType() const
{
	return disambiguatedIdentifier->getType();
}


bool jitcat::AST::CatIdentifier::isAssignable() const
{
	return disambiguatedIdentifier->isAssignable();
}


const CatGenericType& jitcat::AST::CatIdentifier::getAssignableType() const
{
	if (isAssignable())
	{
		return static_cast<CatAssignableExpression*>(disambiguatedIdentifier.get())->getAssignableType();
	}
	return CatGenericType::unknownType;
}


void CatIdentifier::print() const
{
	CatLog::log(name.c_str());
}


bool CatIdentifier::isConst() const
{
	return disambiguatedIdentifier->isConst();
}


CatTypedExpression* CatIdentifier::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	ASTHelper::updatePointerIfChanged(disambiguatedIdentifier, disambiguatedIdentifier->constCollapse(compileTimeContext, errorManager, errorContext));
	return disambiguatedIdentifier.release();
}


CatASTNodeType CatIdentifier::getNodeType() const
{
	return CatASTNodeType::Identifier;
}


std::any CatIdentifier::execute(CatRuntimeContext* runtimeContext)
{
	return disambiguatedIdentifier->execute(runtimeContext);
}


std::any CatIdentifier::executeAssignable(CatRuntimeContext* runtimeContext)
{
	return static_cast<CatAssignableExpression*>(disambiguatedIdentifier.get())->executeAssignable(runtimeContext);
}


bool CatIdentifier::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	std::string lowerName = Tools::toLowerCase(name);
	CatScopeID scopeId = InvalidScopeID;
	if (compiletimeContext->findVariable(lowerName, scopeId) != nullptr)
	{
		//Member variable
		disambiguatedIdentifier = std::make_unique<CatMemberAccess>(new CatScopeRoot(scopeId, lexeme), name, lexeme);
	}
	else if (compiletimeContext->findStaticVariable(lowerName, scopeId) != nullptr)
	{
		//Static variable
		disambiguatedIdentifier = std::make_unique<CatStaticMemberAccess>(new CatStaticScope(true, nullptr, compiletimeContext->getScopeType(scopeId)->getTypeName(), lexeme, lexeme), lexeme, lexeme);
	}
	else if (StaticConstMemberInfo* staticConst = compiletimeContext->findStaticConstant(lowerName, scopeId); staticConst != nullptr)
	{
		//Static constant
		disambiguatedIdentifier = std::make_unique<CatLiteral>(staticConst->getValue(), staticConst->getType(), getLexeme());
	}
	if (disambiguatedIdentifier != nullptr)
	{
		return disambiguatedIdentifier->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	else
	{
		errorManager->compiledWithError(std::string("Variable not found: ") + name, errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
}
