/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#pragma once

#include "jitcat/Configuration.h"
#include "jitcat/TypeConversionCastHelper.h"
#include "jitcat/TypeTraits.h"


namespace jitcat::Reflection
{
	template<typename ArgumentT>
	inline void MemberFunctionInfo::addParameterTypeInfo()
	{
		argumentTypes.push_back(TypeTraits<typename RemoveConst<ArgumentT>::type >::toGenericType());
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::MemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(ClassT::* function)(TFunctionArguments...)) :
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ((void)addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		//To silence unused variable warnings.
		(void)dummy;
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::any MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters)
	{
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::size_t MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getNumberOfArguments() const
	{
		return sizeof...(TFunctionArguments);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline ReturnT MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::staticExecute(ClassT* base, MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments ...args)
	{
		ReturnT(ClassT:: * function)(TFunctionArguments...) = functionInfo->function;
		return (base->*function)(args...);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline MemberFunctionCallData MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getFunctionAddress() const
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


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	template<std::size_t ...Is>
	inline std::any MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>)
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


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::ConstMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(ClassT::* function)(TFunctionArguments...) const) :
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ((void)addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		//To silence unused variable warnings.
		(void)dummy;
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::any ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters)
	{
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::size_t ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getNumberOfArguments() const
	{
		return sizeof...(TFunctionArguments);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline ReturnT ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::staticExecute(ClassT* base, ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments ...args)
	{
		ReturnT(ClassT:: * function)(TFunctionArguments...) const = functionInfo->function;;
		return (base->*function)(args...);
	}


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline MemberFunctionCallData ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getFunctionAddress() const
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


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	template<std::size_t ...Is>
	inline std::any ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const
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


	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::PseudoMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(*function)(ClassT*, TFunctionArguments...)) :
		MemberFunctionInfo(memberFunctionName, TypeTraits<ReturnT>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ((void)addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		//To silence unused variable warnings.
		(void)dummy;
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::any PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters)
	{
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, base, BuildIndices<sizeof...(TFunctionArguments)>{});
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline std::size_t PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getNumberOfArguments() const
	{
		return sizeof...(TFunctionArguments);
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline ReturnT PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::staticExecute(ClassT* base, PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments ...args)
	{
		ReturnT(*function)(ClassT*, TFunctionArguments...) = functionInfo->function;
		return (*function)(base, args...);
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	inline MemberFunctionCallData PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::getFunctionAddress() const
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return MemberFunctionCallData(pointer, reinterpret_cast<uintptr_t>(this), MemberFunctionCallType::PseudoMemberCall);
	}

	template<typename ClassT, typename ReturnT, class ...TFunctionArguments>
	template<std::size_t ...Is>
	inline std::any PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>::callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>)
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
} //End namespace jitcat::Reflection
