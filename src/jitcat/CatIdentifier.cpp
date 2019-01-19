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
	source(RootTypeSource::None)
{
	std::string lowerName = Tools::toLowerCase(name);
	if (context != nullptr)
	{
		//First check if the variable name is a custom local
		findIdentifier(context->getCustomThisType(), RootTypeSource::CustomThis, lowerName);
		//Next, if the variable is not a custom local, check if the variable name is a normal local (via reflection)
		findIdentifier(context->getThisType(), RootTypeSource::This, lowerName);
		//Next, if the variable is not a local, check if the variable name is a custom global
		findIdentifier(context->getCustomGlobalsType(), RootTypeSource::CustomGlobals, lowerName);
		//Lastly, if the variable is not a local, check if the variable name is a global
		findIdentifier(context->getGlobalType(), RootTypeSource::Global, lowerName);
	}

	if (memberInfo != nullptr)
	{
		type = memberInfo->toGenericType();
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
		return memberInfo->isConst;
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
		Reflectable* rootObject = runtimeContext->getRootReference(source);
		std::any rootAny(rootObject);
		return memberInfo->getMemberReference(rootAny);
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


RootTypeSource CatIdentifier::getSource() const
{
	return source;
}


const TypeMemberInfo* CatIdentifier::getMemberInfo() const
{
	return memberInfo;
}


void CatIdentifier::findIdentifier(TypeInfo* typeInfo, RootTypeSource typeSource, const std::string& lowercaseName)
{
	if (memberInfo == nullptr && typeInfo != nullptr)
	{
		memberInfo = typeInfo->getMemberInfo(lowercaseName);
		if (memberInfo != nullptr)
		{
			source = typeSource;
		}
	}
}