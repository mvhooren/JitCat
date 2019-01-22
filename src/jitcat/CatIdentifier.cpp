/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatIdentifier.h"
#include "CatLog.h"
#include "CatRuntimeContext.h"
#include "CustomTypeInfo.h"
#include "Tools.h"

#include <cassert>


CatIdentifier::CatIdentifier(const std::string& name, CatRuntimeContext* context):
	name(name),
	compileTimeContext(context),
	memberInfo(nullptr),
	scopeId(-1)
{
	std::string lowerName = Tools::toLowerCase(name);
	if (context != nullptr)
	{
		memberInfo = context->findVariable(lowerName, scopeId);
	}

	if (memberInfo != nullptr)
	{
		type = memberInfo->catType;
	}
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
	if (memberInfo != nullptr && runtimeContext != nullptr)
	{
		Reflectable* rootObject = runtimeContext->getScopeObject(scopeId);
		return memberInfo->getMemberReference(rootObject);
	}
	assert(false);
	return std::any();
}


CatGenericType CatIdentifier::typeCheck()
{
	if (type.isValidType())
	{
		return type;
	}
	else
	{
		return CatGenericType(std::string("Variable not found: ") + name);
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