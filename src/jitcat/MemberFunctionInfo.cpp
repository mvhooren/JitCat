/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/MemberInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


jitcat::Reflection::MemberFunctionInfo::MemberFunctionInfo(const std::string& memberFunctionName, const CatGenericType& returnType) :
	memberFunctionName(memberFunctionName),
	lowerCaseMemberFunctionName(Tools::toLowerCase(memberFunctionName)),
	returnType(returnType),
	visibility(MemberVisibility::Public)
{}


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


const std::string& jitcat::Reflection::MemberFunctionInfo::getLowerCaseFunctionName() const
{
	return lowerCaseMemberFunctionName;
}


int jitcat::Reflection::MemberFunctionInfo::getNumParameters() const
{
	return (int)argumentTypes.size();
}


const CatGenericType& jitcat::Reflection::MemberFunctionInfo::getParameterType(int index) const
{
	return argumentTypes[index];
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
	std::any baseReferenceValue = baseMember->getMemberReference(reinterpret_cast<unsigned char*>(baseMember->catType.getRawPointer(base)));
	return deferredFunction->call(runtimeContext, baseReferenceValue, parameters);
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
