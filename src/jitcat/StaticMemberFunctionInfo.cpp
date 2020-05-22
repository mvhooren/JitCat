/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "jitcat/StaticMemberFunctionInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;

StaticFunctionInfo::StaticFunctionInfo(const std::string& memberFunctionName, const CatGenericType& returnType): 
			memberFunctionName(memberFunctionName), 
			lowerCaseFunctionName(Tools::toLowerCase(memberFunctionName)),
			returnType(returnType),
			visibility(MemberVisibility::Public)
{}


MemberVisibility StaticFunctionInfo::getVisibility() const
{
	return visibility;
}


const CatGenericType& jitcat::Reflection::StaticFunctionInfo::getArgumentType(std::size_t argumentIndex) const
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

const std::string& jitcat::Reflection::StaticFunctionInfo::getNormalFunctionName() const
{
	return memberFunctionName;
}
