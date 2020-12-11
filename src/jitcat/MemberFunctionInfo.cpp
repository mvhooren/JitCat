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


const CatGenericType& jitcat::Reflection::MemberFunctionInfo::getReturnType() const
{
	return returnType;
}


void MemberFunctionInfo::setReturnType(const CatGenericType& newReturnType)
{
	returnType = newReturnType;
}


MemberVisibility MemberFunctionInfo::getVisibility() const
{
	return visibility;
}


void MemberFunctionInfo::setVisibility(MemberVisibility newVisibility)
{
	visibility = newVisibility;
}


const std::string& MemberFunctionInfo::getMemberFunctionName() const
{
	return memberFunctionName;
}


const std::vector<CatGenericType>& MemberFunctionInfo::getArgumentTypes() const
{
	return argumentTypes;
}


const CatGenericType& MemberFunctionInfo::getArgumentType(std::size_t argumentIndex) const
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


std::string jitcat::Reflection::MemberFunctionInfo::getMangledName() const
{
	return memberFunctionName;
}


void MemberFunctionInfo::addParameterType(const CatGenericType& type)
{
	argumentTypes.push_back(type);
}


DeferredMemberFunctionInfo::DeferredMemberFunctionInfo(TypeMemberInfo* baseMember, MemberFunctionInfo* deferredFunction):
	MemberFunctionInfo(deferredFunction->getMemberFunctionName(), deferredFunction->getReturnType()),
	baseMember(baseMember), deferredFunction(deferredFunction)
{
}


TypeMemberInfo* DeferredMemberFunctionInfo::getBaseMember() const
{
	return baseMember;
}


MemberFunctionInfo* DeferredMemberFunctionInfo::getDeferredFunction() const
{
	return deferredFunction;
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


std::string jitcat::Reflection::DeferredMemberFunctionInfo::getMangledName() const
{
	return deferredFunction->getMangledName();
}
