/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"
#include "jitcat/MemberTypeInfoCreator.h"

namespace jitcat::Reflection
{

template <typename T, typename U>
TypeInfo& TypeInfo::addMember(const std::string& identifier_, U T::* member, MemberFlags flags)
{
	std::string identifier = Tools::toLowerCase(identifier_);
	bool isConst = (flags & MF::isConst) != 0
				    || (flags & MF::isStaticConst) != 0;
	bool isWritable = (flags & MF::isWritable) != 0;
	TypeMemberInfo* memberInfo = MemberTypeInfoCreator<U>::getMemberInfo(identifier_, member, isConst, isWritable);
	if (memberInfo != nullptr)
	{
		members.emplace(identifier, memberInfo);
		if (memberInfo->catType.isPointerToReflectableObjectType() && Tools::startsWith(identifier, "$"))
		{
			addDeferredMembers(memberInfo);
		}
	}
	return *this;
}


template <typename T, typename U, typename ... Args>
TypeInfo& TypeInfo::addMember(const std::string& identifier_, U (T::*function)(Args...))
{
	std::string identifier = Tools::toLowerCase(identifier_);
	if constexpr (!std::is_void<U>::value)
	{
		memberFunctions.emplace(identifier, new MemberFunctionInfoWithArgs<T, U, Args...>(identifier_, function));
	}
	else
	{
		memberFunctions.emplace(identifier, new MemberVoidFunctionInfoWithArgs<T, Args...>(identifier_, function));
	}
	return *this;
}


template <typename T, typename U, typename ... Args>
TypeInfo& TypeInfo::addMember(const std::string& identifier_, U (T::*function)(Args...) const)
{
	std::string identifier = Tools::toLowerCase(identifier_);
	if constexpr (!std::is_void<U>::value)
	{
		memberFunctions.emplace(identifier, new ConstMemberFunctionInfoWithArgs<T, U, Args...>(identifier_, function));
	}
	else
	{
		memberFunctions.emplace(identifier, new ConstMemberVoidFunctionInfoWithArgs<T, Args...>(identifier_, function));
	}
	return *this;
}


} //End namespace jitcat::Reflection