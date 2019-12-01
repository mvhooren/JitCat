/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/BuildIndicesHelper.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/Configuration.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/MemberVisibility.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTraits.h"

#include <string>
#include <vector>

namespace jitcat
{
	class CatRuntimeContext;
	
}

namespace jitcat::Reflection
{
	struct TypeMemberInfo;
	struct DeferredMemberFunctionInfo;

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


	struct MemberFunctionInfo: public FunctionSignature
	{
		MemberFunctionInfo(const std::string& memberFunctionName, const CatGenericType& returnType): 
			memberFunctionName(memberFunctionName), 
			lowerCaseMemberFunctionName(Tools::toLowerCase(memberFunctionName)),
			returnType(returnType), 
			visibility(MemberVisibility::Public)
		{};
		virtual ~MemberFunctionInfo() {}
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) { return std::any(); }
		virtual std::size_t getNumberOfArguments() const { return argumentTypes.size(); }
		inline virtual MemberFunctionCallData getFunctionAddress() const {return MemberFunctionCallData();}
		inline virtual bool isDeferredFunctionCall() {return false;}

		template<typename ParameterT>
		inline void addParameterTypeInfo()
		{
			argumentTypes.push_back(TypeTraits<typename std::remove_cv<ParameterT>::type >::toGenericType());
		}

		CatGenericType getArgumentType(std::size_t argumentIndex) const;

		DeferredMemberFunctionInfo* toDeferredMemberFunction(TypeMemberInfo* baseMember);


		std::string memberFunctionName;
		std::string lowerCaseMemberFunctionName;
		CatGenericType returnType;
		MemberVisibility visibility;

		std::vector<CatGenericType> argumentTypes;

		// Inherited via FunctionSignature
		virtual const std::string& getLowerCaseFunctionName() const override final;
		virtual int getNumParameters() const override final; 
		virtual const CatGenericType& getParameterType(int index) const override final;
	};

	struct DeferredMemberFunctionInfo : public MemberFunctionInfo
	{
		DeferredMemberFunctionInfo(TypeMemberInfo* baseMember, MemberFunctionInfo* deferredFunction);

		virtual ~DeferredMemberFunctionInfo();
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters);
		virtual std::size_t getNumberOfArguments() const;
		inline virtual MemberFunctionCallData getFunctionAddress() const;
		inline virtual bool isDeferredFunctionCall() override final;

		TypeMemberInfo* baseMember;
		MemberFunctionInfo* deferredFunction;

	};



#ifdef _MSC_VER
	#pragma warning (disable:4189)
#endif

template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
struct MemberFunctionInfoWithArgs: public MemberFunctionInfo
{
	MemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT (ClassT::*function)(TFunctionArguments...)):
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
	}


	inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>)
	{
		ClassT* baseObject = std::any_cast<ClassT*>(base);
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			return TypeTraits<ReturnT>::getCatValue((baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename std::remove_cv<TFunctionArguments>::type>::getValueType >(TypeTraits<typename std::remove_cv<TFunctionArguments>::type>::getValue(parameters[Is]))...));
		}
		return TypeTraits<ReturnT>::toGenericType().createDefault();;
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static ReturnT staticExecute(ClassT* base, MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		ReturnT (ClassT::*function)(TFunctionArguments...) = functionInfo->function;;
		return (base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(reinterpret_cast<uintptr_t>(&staticExecute), pointer, reinterpret_cast<uintptr_t>(this), sizeof(function) == Configuration::basicMemberFunctionPointerSize);
	}

	ReturnT (ClassT::*function)(TFunctionArguments...);
};


template <typename ClassT, class ... TFunctionArguments>
struct MemberVoidFunctionInfoWithArgs: public MemberFunctionInfo
{
	MemberVoidFunctionInfoWithArgs(const std::string& memberFunctionName, void (ClassT::*function)(TFunctionArguments...)):
		MemberFunctionInfo(memberFunctionName, TypeTraits<void>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
	}


	inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>)
	{
		ClassT* baseObject = std::any_cast<ClassT*>(base);
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			(baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename std::remove_cv<TFunctionArguments>::type>::getValueType >(TypeTraits<typename std::remove_cv<TFunctionArguments>::type>::getValue(parameters[Is]))...);
		}
		return TypeTraits<void>::toGenericType().createDefault();
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static void staticExecute(ClassT* base, MemberVoidFunctionInfoWithArgs<ClassT, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		void (ClassT::*function)(TFunctionArguments...) = functionInfo->function;
		(base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const  override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(reinterpret_cast<uintptr_t>(&staticExecute), pointer, reinterpret_cast<uintptr_t>(this), sizeof(function) == Configuration::basicMemberFunctionPointerSize);
	}

	void (ClassT::*function)(TFunctionArguments...);
};


template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
struct ConstMemberFunctionInfoWithArgs: public MemberFunctionInfo
{
	ConstMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT (ClassT::*function)(TFunctionArguments...) const):
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
	}


	inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const
	{
		ClassT* baseObject = std::any_cast<ClassT*>(base);
		if (baseObject != nullptr)
		{
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			return TypeTraits<ReturnT>::getCatValue((baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename std::remove_cv<TFunctionArguments>::type>::getValueType >(TypeTraits<typename std::remove_cv<TFunctionArguments>::type>::getValue(parameters[Is]))...));
		}
		return TypeTraits<ReturnT>::toGenericType().createDefault();
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static ReturnT staticExecute(ClassT* base, ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		ReturnT (ClassT::*function)(TFunctionArguments...) const = functionInfo->function;;
		return (base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(reinterpret_cast<uintptr_t>(&staticExecute), pointer, reinterpret_cast<uintptr_t>(this), sizeof(function) == Configuration::basicMemberFunctionPointerSize);
	}

	ReturnT (ClassT::*function)(TFunctionArguments...) const;
};


template <typename ClassT, class ... TFunctionArguments>
struct ConstMemberVoidFunctionInfoWithArgs: public MemberFunctionInfo
{
	ConstMemberVoidFunctionInfoWithArgs(const std::string& memberFunctionName, void (ClassT::*function)(TFunctionArguments...) const):
		MemberFunctionInfo(memberFunctionName, TypeTraits<void>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
	}


	inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const
	{
		ClassT* baseObject = std::any_cast<ClassT*>(base);
		if (baseObject != nullptr)
		{
			
			//This calls the member function, expanding the argument list from the catvalue array
			//std::decay removes const and & from the type.
			(baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename std::decay<TFunctionArguments>::type>::getValueType >(TypeTraits<typename std::decay<TFunctionArguments>::type>::getValue(parameters[Is]))...);
		}
		return TypeTraits<void>::toGenericType().createDefault();
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static void staticExecute(ClassT* base, ConstMemberVoidFunctionInfoWithArgs<ClassT, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		void (ClassT::*function)(TFunctionArguments...) const = functionInfo->function;
		(base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(reinterpret_cast<uintptr_t>(&staticExecute), pointer, reinterpret_cast<uintptr_t>(this), sizeof(function) == Configuration::basicMemberFunctionPointerSize);
	}

	void (ClassT::*function)(TFunctionArguments...) const;
};


} //End namespace jitcat::Reflection