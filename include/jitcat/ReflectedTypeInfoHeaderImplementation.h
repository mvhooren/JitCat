/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/StaticMemberInfo.h"
#include "jitcat/StaticMemberFunctionInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeRegistry.h"
#include "jitcat/TypeTraits.h"
#include "jitcat/MemberTypeInfoCreator.h"
#include "jitcat/ReflectedTypeInfo.h"


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
			TypeInfo::addMember(identifier, memberInfo);
			if (memberInfo->catType.isPointerToReflectableObjectType() && Tools::startsWith(identifier, "$"))
			{
				addDeferredMembers(memberInfo);
			}
		}
		return *this;
	}


	template<typename MemberCVT>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, MemberCVT* member, MemberFlags flags)
	{
		typedef typename RemoveConst<MemberCVT>::type MemberT;
		std::string identifier = Tools::toLowerCase(identifier_);
		bool isConst = (flags & MF::isConst) != 0
						|| (flags & MF::isStaticConst) != 0;
		bool isWritable = (flags & MF::isWritable) != 0;
		if constexpr (std::is_const<MemberCVT>::value)
		{
			isWritable = false;
			isConst = true;
		}
		StaticMemberInfo* memberInfo = nullptr;
		if constexpr (std::is_same<MemberT, float>::value
					  || std::is_same<MemberT, int>::value
					  || std::is_same<MemberT, bool>::value
					  || std::is_same<MemberT, std::string>::value)
		{
			memberInfo = new StaticBasicTypeMemberInfo(identifier, const_cast<MemberT*>(member), TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (TypeTraits<MemberT>::isSerialisableContainer())
		{
			memberInfo = new StaticContainerMemberInfo<MemberT>(identifier, const_cast<MemberT*>(member), TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (TypeTraits<MemberT>::isUniquePtr())
		{
			memberInfo = new StaticClassUniquePtrMemberInfo<typename TypeTraits<MemberT>::type>(identifier, const_cast<MemberT*>(member), TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (std::is_same<MemberT, ReflectableHandle>::value)
		{
			memberInfo = new StaticClassHandleMemberInfo<MemberT>(identifier, const_cast<MemberT*>(member), TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (std::is_pointer<MemberT>::value)
		{
			memberInfo = new StaticClassPointerMemberInfo(identifier, reinterpret_cast<unsigned char**>(const_cast<MemberT*>(member)), TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (std::is_class<MemberT>::value)
		{
			memberInfo = new StaticClassObjectMemberInfo(identifier, reinterpret_cast<unsigned char*>(const_cast<MemberT*>(member)), TypeTraits<MemberT>::toGenericType());
		}
		else if constexpr (std::is_enum<MemberT>::value)
		{
			return addMember(identifier_, reinterpret_cast<typename std::underlying_type_t<MemberT>*>(const_cast<MemberT*>(member)), flags);
		}
		else
		{
			static_assert(false, "Static member type not supported.");
		}
		staticMembers.emplace(identifier, memberInfo);
		return *this;
	}


	template <typename ReflectedT, typename ReturnT, typename ... Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, ReturnT (ReflectedT::*function)(Args...))
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		if constexpr (!std::is_void<ReturnT>::value)
		{
			memberFunctions.emplace(identifier, new MemberFunctionInfoWithArgs<ReflectedT, ReturnT, Args...>(identifier_, function));
		}
		else
		{
			memberFunctions.emplace(identifier, new MemberVoidFunctionInfoWithArgs<ReflectedT, Args...>(identifier_, function));
		}
		return *this;
	}


	template <typename ReflectedT, typename ReturnT, typename ... Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, ReturnT (ReflectedT::*function)(Args...) const)
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		if constexpr (!std::is_void<ReturnT>::value)
		{
			memberFunctions.emplace(identifier, new ConstMemberFunctionInfoWithArgs<ReflectedT, ReturnT, Args...>(identifier_, function));
		}
		else
		{
			memberFunctions.emplace(identifier, new ConstMemberVoidFunctionInfoWithArgs<ReflectedT, Args...>(identifier_, function));
		}
		return *this;
	}
	

	template <typename ReturnT, typename ... Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, ReturnT (*function)(Args...))
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		staticFunctions.emplace(identifier, new StaticFunctionInfoWithArgs<ReturnT, Args...>(identifier_, function));
		return *this;
	}

}

