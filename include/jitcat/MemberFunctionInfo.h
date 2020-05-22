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
#include "jitcat/TypeConversionCastHelper.h"
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

	enum class MemberFunctionCallType
	{
		ThisCall,
		ThisCallThroughStaticFunction,
		PseudoMemberCall,
		Unknown
	};

	struct MemberFunctionCallData
	{
		MemberFunctionCallData(): functionAddress(0), functionInfoStructAddress(0), callType(MemberFunctionCallType::Unknown) {}
		MemberFunctionCallData(uintptr_t functionAddress, uintptr_t functionInfoStructAddress, MemberFunctionCallType callType): 
			functionAddress(functionAddress), 
			functionInfoStructAddress(functionInfoStructAddress), 
			callType(callType)
		{}
		uintptr_t functionAddress;
		uintptr_t functionInfoStructAddress;
		MemberFunctionCallType callType;
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

		template<typename ArgumentT>
		inline void addParameterTypeInfo()
		{
			argumentTypes.push_back(TypeTraits<typename RemoveConst<ArgumentT>::type >::toGenericType());
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
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final;
		virtual std::size_t getNumberOfArguments() const override final;
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		inline virtual bool isDeferredFunctionCall() override final;

		TypeMemberInfo* baseMember;
		MemberFunctionInfo* deferredFunction;

	};



#ifdef _MSC_VER
	//#pragma warning (disable:4189)
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
		//To silence unused variable warnings.
		(void)dummy;
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
			if constexpr (std::is_void_v<ReturnT>)
			{
				(baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...);
			}
			else
			{
				return TypeTraits<ReturnT>::getCatValue((baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...));
			}
			
		}
		return TypeTraits<ReturnT>::toGenericType().createDefault();
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static ReturnT staticExecute(ClassT* base, MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		ReturnT (ClassT::*function)(TFunctionArguments...) = functionInfo->function;
		return (base->*function)(args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const override final
	{
		uintptr_t functionPtr = 0;
		MemberFunctionCallType callType = MemberFunctionCallType::Unknown;
		if constexpr (sizeof(function) == Configuration::basicMemberFunctionPointerSize)
		{
			memcpy(&functionPtr, &function, sizeof(uintptr_t));
			callType = MemberFunctionCallType::ThisCall;
		}
		else
		{
			functionPtr = reinterpret_cast<uintptr_t>(&staticExecute);
			callType = MemberFunctionCallType::ThisCallThroughStaticFunction;
		}
		return MemberFunctionCallData(functionPtr, reinterpret_cast<uintptr_t>(this), callType);
	}

	ReturnT (ClassT::*function)(TFunctionArguments...);
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
		//To silence unused variable warnings.
		(void)dummy;
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
			if constexpr (std::is_void_v<ReturnT>)
			{
				(baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...);
			}
			else
			{
				return TypeTraits<ReturnT>::getCatValue((baseObject->*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...));
			}
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
		uintptr_t functionPtr = 0;
		MemberFunctionCallType callType = MemberFunctionCallType::Unknown;
		if constexpr (sizeof(function) == Configuration::basicMemberFunctionPointerSize)
		{
			memcpy(&functionPtr, &function, sizeof(uintptr_t));
			callType = MemberFunctionCallType::ThisCall;
		}
		else
		{
			functionPtr = reinterpret_cast<uintptr_t>(&staticExecute);
			callType = MemberFunctionCallType::ThisCallThroughStaticFunction;
		}
		return MemberFunctionCallData(functionPtr, reinterpret_cast<uintptr_t>(this), callType);
	}

	ReturnT (ClassT::*function)(TFunctionArguments...) const;
};


template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
struct PseudoMemberFunctionInfoWithArgs: public MemberFunctionInfo
{
	PseudoMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT (*function)(ClassT*, TFunctionArguments...)):
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		//To silence unused variable warnings.
		(void)dummy;
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
			if constexpr (std::is_void_v<ReturnT>)
			{
				(*function)(baseObject, TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...);
			}
			else
			{
				return TypeTraits<ReturnT>::getCatValue((*function)(baseObject, TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...));
			}
			
		}
		return TypeTraits<ReturnT>::toGenericType().createDefault();
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	static ReturnT staticExecute(ClassT* base, PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args)
	{
		ReturnT (*function)(ClassT*, TFunctionArguments...) = functionInfo->function;
		return (*function)(base, args...);
	}


	inline virtual MemberFunctionCallData getFunctionAddress() const override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(pointer, reinterpret_cast<uintptr_t>(this), MemberFunctionCallType::PseudoMemberCall);
	}

	ReturnT (*function)(ClassT*, TFunctionArguments...);
};

} //End namespace jitcat::Reflection
