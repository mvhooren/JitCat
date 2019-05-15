/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/MemberInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


CatGenericType MemberFunctionInfo::getArgumentType(std::size_t argumentIndex) const
{
	if (argumentIndex < argumentTypes.size())
	{
		return argumentTypes[argumentIndex];
	}
	else
	{
		return CatGenericType();
	}
}


DeferredMemberFunctionInfo* MemberFunctionInfo::toDeferredMemberFunction(TypeMemberInfo* baseMember)
{
	return new DeferredMemberFunctionInfo(baseMember, this);
}


DeferredMemberFunctionInfo::DeferredMemberFunctionInfo(TypeMemberInfo* baseMember, MemberFunctionInfo* deferredFunction):
	MemberFunctionInfo(deferredFunction->memberFunctionName, deferredFunction->returnType),
	baseMember(baseMember), deferredFunction(deferredFunction)
{
}


DeferredMemberFunctionInfo::~DeferredMemberFunctionInfo()
{
}


inline std::any DeferredMemberFunctionInfo::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters)
{
	return deferredFunction->call(runtimeContext, baseMember->getMemberReference(std::any_cast<Reflectable*>(base)), parameters);
}


std::size_t DeferredMemberFunctionInfo::getNumberOfArguments() const
{
	return deferredFunction->getNumberOfArguments();
}


inline MemberFunctionCallData DeferredMemberFunctionInfo::getFunctionAddress() const
{
	return deferredFunction->getFunctionAddress();
}


inline bool DeferredMemberFunctionInfo::isDeferredFunctionCall()
{
	return true;
}