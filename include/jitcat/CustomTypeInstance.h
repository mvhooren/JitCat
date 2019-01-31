/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CustomTypeInfo;
struct TypeMemberInfo;
#include "AssignableType.h"
#include "Reflectable.h"

#include <any>
#include <optional>

//Represents an instance of a custom defined type.
//The type is defined by the CustomTypeInfo
//The CustomTypeInfo will create the memory for holding the members. This is stored in data.
//When the instance is destroyed, the data will also be destructud by CustomTypeInfo
class CustomTypeInstance: public Reflectable
{
protected:
	friend class CustomTypeInfo;
	CustomTypeInstance(CustomTypeInfo* typeInfo);
	CustomTypeInstance(unsigned char* data, CustomTypeInfo* typeInfo);
	CustomTypeInstance(const CustomTypeInstance& typeInfo) = delete;

public:
	template<typename MemberT>
	std::optional<MemberT> getMemberValue(const std::string& memberName);

	template <typename MemberT>
	bool setMemberValue(const std::string& memberName, const MemberT& value);

	TypeMemberInfo* getMemberInfo(const std::string& memberName) const;
	std::any getValue(TypeMemberInfo* memberInfo);
	std::optional<std::any> getMemberAnyValue(const std::string& memberName);
	std::optional<std::any> getMemberAnyValueReference(const std::string& memberName, AssignableType& assignableType);

public:
	virtual ~CustomTypeInstance();

	CustomTypeInfo* typeInfo;
	unsigned char* data;
};


#include "ASTHelper.h"
#include "TypeTraits.h"

template<typename MemberT>
std::optional<MemberT> CustomTypeInstance::getMemberValue(const std::string& memberName)
{
	auto result = getMemberAnyValue(memberName);
	if (result.has_value())
	{
		return TypeTraits<MemberT>::getValue(*result);
	}
	return std::optional<MemberT>();
}



template<typename MemberT>
inline bool CustomTypeInstance::setMemberValue(const std::string& memberName, const MemberT& value)
{
	AssignableType type = AssignableType::None;
	auto result = getMemberAnyValueReference(memberName, type);
	if (result.has_value())
	{
		TypeMemberInfo* memberInfo = getMemberInfo(memberName);
		ASTHelper::doAssignment(*result, TypeTraits<MemberT>::getCatValue(value), memberInfo->catType, type);
		return true;
	}
	return false;
}
