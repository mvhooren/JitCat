/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/AssignableType.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/Reflectable.h"
#include "jitcat/TypeTraits.h"

#include <any>
#include <optional>


namespace jitcat::Reflection
{
	class CustomTypeInfo;
	struct TypeMemberInfo;


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
			std::any anyValue = TypeTraits<MemberT>::getCatValue(value);
			jitcat::AST::ASTHelper::doAssignment(*result, anyValue, memberInfo->catType, type);
			return true;
		}
		return false;
	}


} //End namespace jitcat::Reflection