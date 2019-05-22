/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatScopeRoot.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"

using namespace jitcat;
using namespace jitcat::AST;


CatScopeRoot::CatScopeRoot(CatScopeID scopeId, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	scopeId(scopeId),
	type(CatGenericType::errorType)
{
}


jitcat::AST::CatScopeRoot::CatScopeRoot(const CatScopeRoot& other):
	CatTypedExpression(other),
	scopeId(other.scopeId),
	type(CatGenericType::errorType)
{
}


CatASTNode* jitcat::AST::CatScopeRoot::copy() const
{
	return new CatScopeRoot(*this);
}


void CatScopeRoot::print() const
{
}


CatASTNodeType CatScopeRoot::getNodeType() const
{
	return CatASTNodeType::ScopeRoot;
}


std::any CatScopeRoot::execute(CatRuntimeContext* runtimeContext)
{
	return runtimeContext->getScopeObject(scopeId);
}


bool CatScopeRoot::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	type = CatGenericType::errorType;
	if (compiletimeContext != nullptr)
	{
		type = CatGenericType(compiletimeContext->getScopeType(scopeId), Reflection::TypeOwnershipSemantics::Weak);
	}
	if (type.isValidType())
	{
		return true;
	}
	else
	{
		errorManager->compiledWithError(std::string("Invalid scope."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
}


CatGenericType CatScopeRoot::getType() const
{
	return type;
}


bool CatScopeRoot::isConst() const
{
	return false;
}


CatTypedExpression* CatScopeRoot::constCollapse(CatRuntimeContext* compileTimeContext)
{
	if (compileTimeContext->isStaticScope(scopeId))
	{
		return new CatLiteral(compileTimeContext->getScopeObject(scopeId), type, getLexeme());
	}
	return this;
}


CatScopeID CatScopeRoot::getScopeId() const
{
	return scopeId;
}
