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
TypeInfo& TypeInfo::addMember(const std::string& identifier_, U T::* member, unsigned int flags)
{
	std::string identifier = Tools::toLowerCase(identifier_);
	bool isConst = (flags & MTF_IS_CONST) != 0
				    || (flags & MTF_IS_STATIC_CONST) != 0;
	bool isWritable = (flags & MTF_IS_WRITABLE) != 0;
	TypeMemberInfo* memberInfo = MemberTypeInfoCreator<U>::getMemberInfo(identifier_, member, isConst, isWritable);
	if (memberInfo != nullptr)
	{
		members.emplace(identifier, memberInfo);
	}
	return *this;
}


template <typename T, typename ... Args>
TypeInfo& TypeInfo::addMember(const std::string& identifier_, void (T::*function)(Args...))
{
	std::string identifier = Tools::toLowerCase(identifier_);
	memberFunctions.emplace(identifier, new MemberVoidFunctionInfoWithArgs<T, Args...>(identifier_, function));
	return *this;
}


template <typename T, typename U, typename ... Args>
TypeInfo& TypeInfo::addMember(const std::string& identifier_, U (T::*function)(Args...))
{
	std::string identifier = Tools::toLowerCase(identifier_);
	memberFunctions.emplace(identifier, new MemberFunctionInfoWithArgs<T, U, Args...>(identifier_, function));
	return *this;
}


template <typename T, typename ... Args>
TypeInfo& TypeInfo::addMember(const std::string& identifier_, void (T::*function)(Args...) const)
{
	std::string identifier = Tools::toLowerCase(identifier_);
	memberFunctions.emplace(identifier, new ConstMemberVoidFunctionInfoWithArgs<T, Args...>(identifier_, function));
	return *this;
}


template <typename T, typename U, typename ... Args>
TypeInfo& TypeInfo::addMember(const std::string& identifier_, U (T::*function)(Args...) const)
{
	std::string identifier = Tools::toLowerCase(identifier_);
	memberFunctions.emplace(identifier, new ConstMemberFunctionInfoWithArgs<T, U, Args...>(identifier_, function));
	return *this;
}


} //End namespace jitcat::Reflection