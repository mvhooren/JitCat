/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/TypeTraits.h"
#include "jitcat/TypeConversionCastHelper.h"


namespace jitcat::Reflection
{
	template<typename ArgumentT>
	inline void StaticFunctionInfo::addParameterTypeInfo()
	{
		argumentTypes.push_back(TypeTraits<typename RemoveConst<ArgumentT>::type >::toGenericType());
	}

	template<typename ReturnT, class ...TFunctionArguments>
	inline StaticFunctionInfoWithArgs<ReturnT, TFunctionArguments...>::StaticFunctionInfoWithArgs(const std::string& memberFunctionName, TypeInfo* parentType, ReturnT(*function)(TFunctionArguments...)) :
		StaticFunctionInfo(memberFunctionName, parentType, TypeTraits<std::remove_cv_t<ReturnT>>::toGenericType()),
		function(function)
	{
		//Trick to call a function per variadic template item
		//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
		//This gets the type info per parameter type
		int dummy[] = { 0, ((void)addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		//To silence unused variable warnings.
		(void)dummy;
		//Link the function to the pre-compiled expressions
		if constexpr (Configuration::usePreCompiledExpressions)
		{
			JitCat::get()->setPrecompiledLinkedFunction(getMangledFunctionName(), getFunctionAddress());
		}
	}


	template<typename ReturnT, class ...TFunctionArguments>
	inline std::any StaticFunctionInfoWithArgs<ReturnT, TFunctionArguments...>::call(CatRuntimeContext* runtimeContext, const std::vector<std::any>& parameters)
	{
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<typename ReturnT, class ...TFunctionArguments>
	inline std::size_t StaticFunctionInfoWithArgs<ReturnT, TFunctionArguments...>::getNumberOfArguments() const
	{
		return sizeof...(TFunctionArguments);
	}


	template<typename ReturnT, class ...TFunctionArguments>
	inline uintptr_t StaticFunctionInfoWithArgs<ReturnT, TFunctionArguments...>::getFunctionAddress() const
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		static_assert(sizeof(function) == sizeof(uintptr_t), "Unsupported function pointer.");
		return pointer;
	}


	template<typename ReturnT, class ...TFunctionArguments>
	template<std::size_t ...Is>
	inline std::any StaticFunctionInfoWithArgs<ReturnT, TFunctionArguments...>::callWithIndexed(const std::vector<std::any>& parameters, Indices<Is...>)
	{
		if constexpr (std::is_same<void, ReturnT>::value)
		{
			(*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...);
			return std::any();
		}
		else
		{
			return TypeTraits<ReturnT>::getCatValue((*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...));
		}
	}
}