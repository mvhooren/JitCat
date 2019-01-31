/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/Configuration.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTraits.h"

#include <string>
#include <vector>

namespace jitcat::Reflection
{

struct MemberFunctionCallData
{
	MemberFunctionCallData(): staticFunctionAddress(0), memberFunctionAddress(0), functionInfoStructAddress(0), makeDirectCall(false) {}
	MemberFunctionCallData(uintptr_t staticFunctionAddress, uintptr_t memberFunctionAddress, uintptr_t functionInfoStructAddress, bool makeDirectCall): 
		staticFunctionAddress(staticFunctionAddress), 
		memberFunctionAddress(memberFunctionAddress), 
		functionInfoStructAddress(functionInfoStructAddress), 
		makeDirectCall(makeDirectCall) 
	{}
	uintptr_t staticFunctionAddress;
	uintptr_t memberFunctionAddress;
	uintptr_t functionInfoStructAddress;
	bool makeDirectCall;
};


struct MemberFunctionInfo
{
	MemberFunctionInfo(const std::string& memberFunctionName, const CatGenericType& returnType): memberFunctionName(memberFunctionName), returnType(returnType) {};
	virtual ~MemberFunctionInfo() {}
	inline virtual std::any call(std::any& base, const std::vector<std::any>& parameters) { return std::any(); }
	virtual std::size_t getNumberOfArguments() const { return argumentTypes.size(); }
	inline virtual MemberFunctionCallData getFunctionAddress() const {return MemberFunctionCallData();}

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


	inline virtual std::any call(std::any& base, const std::vector<std::any>& parameters) override
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>)
	{
		T* baseObject = static_cast<T*>(std::any_cast<Reflectable*>(base));
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			return TypeTraits<U>::getCatValue((baseObject->*function)(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is])...));
		}
		return TypeTraits<U>::toGenericType().createDefault();;
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static U staticExecute(T* base, MemberFunctionInfoWithArgs<T, U, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		U (T::*function)(TFunctionArguments...) = functionInfo->function;;
		return (base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(reinterpret_cast<uintptr_t>(&staticExecute), pointer, reinterpret_cast<uintptr_t>(this), sizeof(function) == Configuration::basicMemberFunctionPointerSize);
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


	inline virtual std::any call(std::any& base, const std::vector<std::any>& parameters) 
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>)
	{
		T* baseObject = static_cast<T*>(std::any_cast<Reflectable*>(base));
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			(baseObject->*function)(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is])...);
		}
		return TypeTraits<void>::toGenericType().createDefault();
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static void staticExecute(T* base, MemberVoidFunctionInfoWithArgs<T, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		void (T::*function)(TFunctionArguments...) = functionInfo->function;
		(base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const  override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(reinterpret_cast<uintptr_t>(&staticExecute), pointer, reinterpret_cast<uintptr_t>(this), sizeof(function) == Configuration::basicMemberFunctionPointerSize);
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


	inline virtual std::any call(std::any& base, const std::vector<std::any>& parameters)
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const
	{
		T* baseObject = static_cast<T*>(std::any_cast<Reflectable*>(base));
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			return TypeTraits<U>::getCatValue((baseObject->*function)(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is])...));
		}
		return TypeTraits<U>::toGenericType().createDefault();
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static U staticExecute(T* base, ConstMemberFunctionInfoWithArgs<T, U, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		U (T::*function)(TFunctionArguments...) const = functionInfo->function;;
		return (base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(reinterpret_cast<uintptr_t>(&staticExecute), pointer, reinterpret_cast<uintptr_t>(this), sizeof(function) == Configuration::basicMemberFunctionPointerSize);
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


	inline virtual std::any call(std::any& base, const std::vector<std::any>& parameters) 
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const
	{
		T* baseObject = static_cast<T*>(std::any_cast<Reflectable*>(base));
		if (baseObject != nullptr)
		{
			
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			(baseObject->*function)(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is])...);
		}
		return TypeTraits<void>::toGenericType().createDefault();
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static void staticExecute(T* base, ConstMemberVoidFunctionInfoWithArgs<T, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		void (T::*function)(TFunctionArguments...) const = functionInfo->function;
		(base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(reinterpret_cast<uintptr_t>(&staticExecute), pointer, reinterpret_cast<uintptr_t>(this), sizeof(function) == Configuration::basicMemberFunctionPointerSize);
	}

	void (T::*function)(TFunctionArguments...) const;
};


} //End namespace jitcat::Reflection