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


#include <type_traits>


namespace jitcat::Reflection
{

	template<typename ReflectedT, typename MemberT>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, MemberT ReflectedT::* member, MemberFlags flags)
	{
		//Check if a member with this name already exists
		TypeMemberInfo* info = getMemberInfo(identifier_);
		assert(info == nullptr);
		if (info != nullptr)
		{
			return *this;
		}
		std::string identifier = Tools::toLowerCase(identifier_);
		bool isConst = (flags & MF::isConst) != 0
						|| (flags & MF::isStaticConst) != 0;
		bool isWritable = (flags & MF::isWritable) != 0;
		TypeMemberInfo* memberInfo = MemberTypeInfoCreator<MemberT>::getMemberInfo(identifier_, member, isConst, isWritable);
		if (memberInfo != nullptr)
		{
			if ((memberInfo->getType().isReflectableHandleType() || memberInfo->getType().isPointerToReflectableObjectType())
				&& (Tools::startsWith(identifier, "$") || memberInfo->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value))
			{
				memberInfo->getType().getPointeeType()->getObjectType()->addDependentType(this);
			}
			TypeInfo::addMember(identifier, memberInfo);
			if (memberInfo->getType().isPointerToReflectableObjectType() && Tools::startsWith(identifier, "$"))
			{
				addDeferredMembers(memberInfo);
			}
		}
		return *this;
	}


	template<typename MemberCVT>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, MemberCVT* member, MemberFlags flags)
	{
		//Check if a member with this name already exists
		TypeMemberInfo* info = getMemberInfo(identifier_);
		assert(info == nullptr);
		if (info != nullptr)
		{
			return *this;
		}
		using MemberT = typename RemoveConst<MemberCVT>::type;
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
		if constexpr (std::is_floating_point_v<MemberT>
					  || std::is_integral_v<MemberT>
					  || std::is_same<MemberT, bool>::value
					  || std::is_enum<MemberT>::value)
		{
			memberInfo = new StaticBasicTypeMemberInfo<MemberT>(identifier, const_cast<MemberT*>(member), TypeTraits<MemberT>::toGenericType(), getTypeName());
		}
		else if constexpr (TypeTraits<MemberT>::isUniquePtr())
		{
			memberInfo = new StaticClassUniquePtrMemberInfo<typename TypeTraits<MemberT>::type>(identifier, const_cast<MemberT*>(member), TypeTraits<MemberT>::toGenericType(), getTypeName());
		}
		else if constexpr (std::is_same<MemberT, ReflectableHandle>::value)
		{
			CatGenericType handleType = TypeTraits<MemberT>::toGenericType().toHandle(Reflection::TypeOwnershipSemantics::Weak, isWritable, isConst);
			memberInfo = new StaticClassHandleMemberInfo(identifier, const_cast<MemberT*>(member), handleType, getTypeName());
		}
		else if constexpr (std::is_pointer<MemberT>::value)
		{
			memberInfo = new StaticClassPointerMemberInfo(identifier, reinterpret_cast<unsigned char**>(const_cast<MemberT*>(member)), 
														  TypeTraits<MemberT>::toGenericType().toPointer(Reflection::TypeOwnershipSemantics::Weak, isWritable, isConst), 
														  getTypeName());
		}
		else if constexpr (std::is_class<MemberT>::value)
		{
			memberInfo = new StaticClassObjectMemberInfo(identifier, reinterpret_cast<unsigned char*>(const_cast<MemberT*>(member)), 
														 TypeTraits<MemberT>::toGenericType().toPointer(Reflection::TypeOwnershipSemantics::Value, isWritable, isConst), 
														 getTypeName());
		}
		else
		{
			static_assert(std::is_class_v<MemberT>, "Static member type not supported.");
		}
		staticMembers.emplace(identifier, memberInfo);
		return *this;
	}


	template <typename ReflectedT, typename ReturnT, typename ... Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, ReturnT (ReflectedT::*function)(Args...))
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		memberFunctions.emplace(identifier, new MemberFunctionInfoWithArgs<ReflectedT, ReturnT, Args...>(identifier_, function));
		return *this;
	}


	template <typename ReflectedT, typename ReturnT, typename ... Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, ReturnT (ReflectedT::*function)(Args...) const)
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		memberFunctions.emplace(identifier, new ConstMemberFunctionInfoWithArgs<ReflectedT, ReturnT, Args...>(identifier_, function));
		return *this;
	}
	

	template <typename ReturnT, typename ... Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addMember(const std::string& identifier_, ReturnT (*function)(Args...))
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		staticFunctions.emplace(identifier, new StaticFunctionInfoWithArgs<ReturnT, Args...>(identifier_, this, function));
		return *this;
	}


	template<typename ReflectedT, typename ReturnT, typename ...Args>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addPseudoMemberFunction(const std::string& identifier_, ReturnT(*function)(ReflectedT*, Args...))
	{
		std::string identifier = Tools::toLowerCase(identifier_);
		memberFunctions.emplace(identifier, new PseudoMemberFunctionInfoWithArgs<ReflectedT, ReturnT, Args...>(identifier_, function));
		return *this;
	}


	template<typename ConstantT>
	inline ReflectedTypeInfo& ReflectedTypeInfo::addConstant(const std::string& identifier, ConstantT value)
	{
		//Check if a constant with this name already exists
		StaticConstMemberInfo* info = getStaticConstMemberInfo(identifier);
		assert(info == nullptr);
		if (info != nullptr)
		{
			return *this;
		}
		CatGenericType type = TypeTraits<typename RemoveConst<ConstantT>::type>::toGenericType();
		std::any anyValue = TypeTraits<ConstantT>::getCatValue(value);
		TypeInfo::addConstant(identifier, type, anyValue);
		return *this;
	}

}

