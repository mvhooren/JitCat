/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatGenericType.h"
#include "CatValue.h"
#include "MemberReferencePtr.h"
#include "TypeTraits.h"

#include <string>
#include <vector>


struct MemberFunctionInfo
{
	MemberFunctionInfo(const std::string& memberFunctionName, const CatGenericType& returnType): memberFunctionName(memberFunctionName), returnType(returnType) {};
	virtual ~MemberFunctionInfo() {}
	inline virtual CatValue call(MemberReferencePtr& base, const std::vector<CatValue>& parameters) { return CatValue(); }
	virtual std::size_t getNumberOfArguments() const { return argumentTypes.size(); }

	template<typename X>
	inline void addParameterTypeInfo()
	{
		argumentTypes.push_back(TypeTraits<typename std::decay<X>::type >::toGenericType());
	}

	CatGenericType getArgumentType(std::size_t argumentIndex) const 
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

	std::string memberFunctionName;
	CatGenericType returnType;

	std::vector<CatGenericType> argumentTypes;
};


////Indices trick
//https://stackoverflow.com/questions/15014096/c-index-of-type-during-variadic-template-expansion
//Allows getting indices per template in variadic template expansion
template <std::size_t... Is>
struct Indices {};
 
template <std::size_t N, std::size_t... Is>
struct BuildIndices
    : BuildIndices<N-1, N-1, Is...> {};
 
template <std::size_t... Is>
struct BuildIndices<0, Is...> : Indices<Is...> {};

#ifdef _MSC_VER
	#pragma warning (disable:4189)
#endif

template <typename T, typename U, class ... TFunctionArguments>
struct MemberFunctionInfoWithArgs: public MemberFunctionInfo
{
	MemberFunctionInfoWithArgs(const std::string& memberFunctionName, U (T::*function)(TFunctionArguments...)):
		MemberFunctionInfo(memberFunctionName, TypeTraits<U>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
	}


	inline virtual CatValue call(MemberReferencePtr& base, const std::vector<CatValue>& parameters) 
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	CatValue callWithIndexed(const std::vector<CatValue>& parameters, MemberReferencePtr& base, Indices<Is...>)
	{
		if (!base.isNull()
			&& base->getParentObject() != nullptr)
		{
			T* baseObject = static_cast<T*>(base->getParentObject());
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			return TypeTraits<U>::getCatValue((baseObject->*function)(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is])...));
		}
		return CatValue();
	}


	virtual std::size_t getNumberOfArguments() const override 
	{ 
		return sizeof...(TFunctionArguments);
	}

	U (T::*function)(TFunctionArguments...);
};


template <typename T, class ... TFunctionArguments>
struct MemberVoidFunctionInfoWithArgs: public MemberFunctionInfo
{
	MemberVoidFunctionInfoWithArgs(const std::string& memberFunctionName, void (T::*function)(TFunctionArguments...)):
		MemberFunctionInfo(memberFunctionName, TypeTraits<void>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
	}


	inline virtual CatValue call(MemberReferencePtr& base, const std::vector<CatValue>& parameters) 
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	CatValue callWithIndexed(const std::vector<CatValue>& parameters, MemberReferencePtr& base, Indices<Is...>)
	{
		if (!base.isNull()
			&& base->getParentObject() != nullptr)
		{
			T* baseObject = static_cast<T*>(base->getParentObject());
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			(baseObject->*function)(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is])...);
		}
		return CatValue();
	}


	virtual std::size_t getNumberOfArguments() const override 
	{ 
		return sizeof...(TFunctionArguments);
	}

	void (T::*function)(TFunctionArguments...);
};


template <typename T, typename U, class ... TFunctionArguments>
struct ConstMemberFunctionInfoWithArgs: public MemberFunctionInfo
{
	ConstMemberFunctionInfoWithArgs(const std::string& memberFunctionName, U (T::*function)(TFunctionArguments...) const):
		MemberFunctionInfo(memberFunctionName, TypeTraits<U>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
	}


	inline virtual CatValue call(MemberReferencePtr& base, const std::vector<CatValue>& parameters)
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	CatValue callWithIndexed(const std::vector<CatValue>& parameters, MemberReferencePtr& base, Indices<Is...>) const
	{
		if (!base.isNull()
			&& base->getParentObject() != nullptr)
		{
			T* baseObject = static_cast<T*>(base->getParentObject());
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			return TypeTraits<U>::getCatValue((baseObject->*function)(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is])...));
		}
		return CatValue();
	}


	virtual std::size_t getNumberOfArguments() const override 
	{ 
		return sizeof...(TFunctionArguments);
	}

	U (T::*function)(TFunctionArguments...) const;
};


template <typename T, class ... TFunctionArguments>
struct ConstMemberVoidFunctionInfoWithArgs: public MemberFunctionInfo
{
	ConstMemberVoidFunctionInfoWithArgs(const std::string& memberFunctionName, void (T::*function)(TFunctionArguments...) const):
		MemberFunctionInfo(memberFunctionName, TypeTraits<void>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
	}


	inline virtual CatValue call(MemberReferencePtr& base, const std::vector<CatValue>& parameters) 
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	CatValue callWithIndexed(const std::vector<CatValue>& parameters, MemberReferencePtr& base, Indices<Is...>) const
	{
		if (!base.isNull()
			&& base->getParentObject() != nullptr)
		{
			T* baseObject = static_cast<T*>(base->getParentObject());
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			(baseObject->*function)(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is])...);
		}
		return CatValue();
	}


	virtual std::size_t getNumberOfArguments() const override 
	{ 
		return sizeof...(TFunctionArguments);
	}

	void (T::*function)(TFunctionArguments...) const;
};