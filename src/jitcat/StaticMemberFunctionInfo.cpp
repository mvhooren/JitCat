/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "jitcat/StaticMemberFunctionInfo.h"
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::Reflection;

StaticFunctionInfo::StaticFunctionInfo(const std::string& memberFunctionName, TypeInfo* parentType, const CatGenericType& returnType): 
	parentType(parentType),
	memberFunctionName(memberFunctionName), 
	lowerCaseFunctionName(Tools::toLowerCase(memberFunctionName)),
	returnType(returnType),
	visibility(MemberVisibility::Public)
{
}


MemberVisibility StaticFunctionInfo::getVisibility() const
{
	return visibility;
}


void StaticFunctionInfo::addParameter(const CatGenericType& type)
{
	argumentTypes.push_back(type);
}


const CatGenericType& StaticFunctionInfo::getArgumentType(std::size_t argumentIndex) const
{
	if (argumentIndex < argumentTypes.size())
	{
		return argumentTypes[argumentIndex];
	}
	else
	{
		return CatGenericType::unknownType;
	}
}


TypeInfo* Reflection::StaticFunctionInfo::getParentType() const
{
	return parentType;
}


const std::string& StaticFunctionInfo::getLowerCaseFunctionName() const
{
	return lowerCaseFunctionName;
}


int StaticFunctionInfo::getNumParameters() const
{
	return (int)argumentTypes.size();
}


const CatGenericType& StaticFunctionInfo::getParameterType(int index) const
{
	return argumentTypes[index];
}


bool StaticFunctionInfo::getNeverReturnsNull() const
{
	return false;
}


const std::string& StaticFunctionInfo::getNormalFunctionName() const
{
	return memberFunctionName;
}


std::string StaticFunctionInfo::getMangledFunctionName(bool sRetBeforeThis) const
{
	std::string baseName;
	if (parentType != nullptr)
	{
		baseName = parentType->getQualifiedTypeName();
	}
	return FunctionNameMangler::getMangledFunctionName(returnType, memberFunctionName, argumentTypes, false, baseName, sRetBeforeThis);
}
