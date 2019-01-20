/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatScopeRoot.h"
#include "CatRuntimeContext.h"


CatScopeRoot::CatScopeRoot(CatScopeID scopeId, CatRuntimeContext* context):
	scopeId(scopeId)
{
	type = context->getScopeType(scopeId);
}


void CatScopeRoot::print() const
{
}


CatASTNodeType CatScopeRoot::getNodeType()
{
	return CatASTNodeType::ScopeRoot;
}


std::any CatScopeRoot::execute(CatRuntimeContext* runtimeContext)
{
	return runtimeContext->getScopeObject(scopeId);
}


CatGenericType CatScopeRoot::typeCheck()
{
	if (type.isValidType())
	{
		return type;
	}
	else
	{
		return CatGenericType(std::string("Invalid scope."));
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
	return this;
}


CatScopeID CatScopeRoot::getScopeId() const
{
	return scopeId;
}
