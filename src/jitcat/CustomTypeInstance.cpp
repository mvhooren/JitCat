/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CustomTypeInstance.h"
#include "CustomTypeInfo.h"


CustomTypeInstance::CustomTypeInstance(CustomTypeInfo* typeInfo):
	typeInfo(typeInfo)
{
	data = typeInfo->instanceConstructor();
}


CustomTypeInstance::CustomTypeInstance(unsigned char* data, CustomTypeInfo* typeInfo):
	data(data),
	typeInfo(typeInfo)
{
}


TypeMemberInfo* CustomTypeInstance::getMemberInfo(const std::string& memberName) const
{
	return typeInfo->getMemberInfo(memberName);
}


std::any CustomTypeInstance::getValue(TypeMemberInfo* memberInfo)
{
	return memberInfo->getMemberReference(this);
}


std::optional<std::any> CustomTypeInstance::getMemberAnyValue(const std::string& memberName)
{
	TypeMemberInfo* memberInfo = getMemberInfo(memberName);
	if (memberInfo != nullptr)
	{
		return memberInfo->getMemberReference(this);
	}
	return std::optional<std::any>();
}


std::optional<std::any> CustomTypeInstance::getMemberAnyValueReference(const std::string& memberName, AssignableType& assignableType)
{
	TypeMemberInfo* memberInfo = getMemberInfo(memberName);
	if (memberInfo != nullptr)
	{
		return memberInfo->getAssignableMemberReference(this, assignableType);
	}
	return std::optional<std::any>();
}


CustomTypeInstance::~CustomTypeInstance()
{
	typeInfo->instanceDestructor(this);
}
