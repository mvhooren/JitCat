/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/InfixOperatorResultInfo.h"
#include "jitcat/CatGenericType.h"

using namespace jitcat;
using namespace jitcat::AST;


InfixOperatorResultInfo::InfixOperatorResultInfo():
	resultType(new CatGenericType()),
	isOverloaded(false),
	staticOverloadedType(nullptr)
{
}


void InfixOperatorResultInfo::setResultType(const CatGenericType& result)
{
	resultType->operator=(result);
}


const CatGenericType& InfixOperatorResultInfo::getResultType() const
{
	return *resultType.get();
}


void InfixOperatorResultInfo::setIsOverloaded(bool overloaded)
{
	isOverloaded = overloaded;
}


bool InfixOperatorResultInfo::getIsOverloaded() const
{
	return isOverloaded;
}


bool InfixOperatorResultInfo::getIsStaticOverloaded() const
{
	return staticOverloadedType != nullptr;
}


void InfixOperatorResultInfo::setStaticOverloadType(Reflection::TypeInfo* overloadType)
{
	staticOverloadedType = overloadType;
}


Reflection::TypeInfo* InfixOperatorResultInfo::getStaticOverloadedType() const
{
	return staticOverloadedType;
}
