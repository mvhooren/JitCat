/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/StaticMemberInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeRegistry.h"
#include "jitcat/TypeTraits.h"
#include "jitcat/MemberTypeInfoCreator.h"
#include "ReflectedTypeInfo.h"


#include <type_traits>


namespace jitcat::Reflection
{

	template<typename ReflectedT, typename MemberT>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, MemberT ReflectedT::* member, MemberFlags flags)
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		bool isConst = (flags & MF::isConst) != 0
						|| (flags & MF::isStaticConst) != 0;
		bool isWritable = (flags & MF::isWritable) != 0;
		TypeMemberInfo* memberInfo = MemberTypeInfoCreator<MemberT>::getMemberInfo(identifier_, member, isConst, isWritable);
		if (memberInfo != nullptr)
		{
			if ((memberInfo->catType.isReflectableHandleType() || memberInfo->catType.isPointerToReflectableObjectType())
				&& (Tools::startsWith(identifier, "$") || memberInfo->catType.getOwnershipSemantics() == TypeOwnershipSemantics::Value))
			{
				memberInfo->catType.getPointeeType()->getObjectType()->addDependentType(this);
			}
			members.emplace(identifier, memberInfo);
			if (memberInfo->catType.isPointerToReflectableObjectType() && Tools::startsWith(identifier, "$"))
			{
				addDeferredMembers(memberInfo);
			}
		}
		return *this;
	}


	template<typename MemberT>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, MemberT* member, MemberFlags flags)
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		bool isConst = (flags & MF::isConst) != 0
						|| (flags & MF::isStaticConst) != 0;
		bool isWritable = (flags & MF::isWritable) != 0;
		StaticMemberInfo* memberInfo = nullptr;
		if constexpr (std::is_same<MemberT, float>::value
					  || std::is_same<MemberT, int>::value
					  || std::is_same<MemberT, bool>::value
					  || std::is_same<MemberT, std::string>::value)
		{
			memberInfo = new StaticBasicTypeMemberInfo(identifier, member, TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (TypeTraits<MemberT>::isSerialisableContainer())
		{
			memberInfo = new StaticContainerMemberInfo<MemberT>(identifier, member, TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (TypeTraits<MemberT>::isUniquePtr())
		{
			memberInfo = new StaticClassUniquePtrMemberInfo<MemberT>(identifier, member, TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (std::is_same<MemberT, ReflectableHandle>::value)
		{
			memberInfo = new StaticClassHandleMemberInfo<MemberT>(identifier, member, TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (std::is_pointer<MemberT>::value)
		{
			memberInfo = new StaticClassPointerMemberInfo(identifier, member, TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (std::is_class<MemberT>::value)
		{
			memberInfo = new StaticClassObjectMemberInfo(identifier, member, TypeTraits<MemberT>::toGenericType());
		}
		else
		{
			static_assert(false, "Static member type not supported.");
		}
		
		staticMembers.emplace(identifier, memberInfo);
		return *this;
	}


	template <typename ReflectedT, typename MemberT, typename ... Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, MemberT (ReflectedT::*function)(Args...))
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		if constexpr (!std::is_void<MemberT>::value)
		{
			memberFunctions.emplace(identifier, new MemberFunctionInfoWithArgs<ReflectedT, MemberT, Args...>(identifier_, function));
		}
		else
		{
			memberFunctions.emplace(identifier, new MemberVoidFunctionInfoWithArgs<ReflectedT, Args...>(identifier_, function));
		}
		return *this;
	}


	template <typename ReflectedT, typename MemberT, typename ... Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, MemberT (ReflectedT::*function)(Args...) const)
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		if constexpr (!std::is_void<MemberT>::value)
		{
			memberFunctions.emplace(identifier, new ConstMemberFunctionInfoWithArgs<ReflectedT, MemberT, Args...>(identifier_, function));
		}
		else
		{
			memberFunctions.emplace(identifier, new ConstMemberVoidFunctionInfoWithArgs<ReflectedT, Args...>(identifier_, function));
		}
		return *this;
	}

}

