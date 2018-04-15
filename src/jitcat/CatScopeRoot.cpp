/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatScopeRoot.h"
#include "CatRuntimeContext.h"


CatScopeRoot::CatScopeRoot(RootTypeSource source, CatRuntimeContext* context):
	source(source)
{
	switch (source)
	{
		case RootTypeSource::Global:
			type = CatGenericType(context->getGlobalType());
			break;
		case RootTypeSource::This:
			type = CatGenericType(context->getThisType());
			break;
		case RootTypeSource::CustomThis:
			type = CatGenericType(context->getCustomThisType());
			break;
		case RootTypeSource::CustomGlobals:
			type = CatGenericType(context->getCustomGlobalsType());
			break;
	}
}


void CatScopeRoot::print() const
{
}


CatASTNodeType CatScopeRoot::getNodeType()
{
	return CatASTNodeType::ScopeRoot;
}


CatValue CatScopeRoot::execute(CatRuntimeContext* runtimeContext)
{
	return runtimeContext->getRootReference(source);
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
