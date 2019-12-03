/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include "jitcat/BuildIndicesHelper.h"
#include "jitcat/CatGenericType.h"
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
	class StaticFunctionInfo: public FunctionSignature
	{
	public:
		StaticFunctionInfo(const std::string& memberFunctionName, const CatGenericType& returnType);
		virtual ~StaticFunctionInfo() {}
		inline virtual std::any call(CatRuntimeContext* runtimeContext, const std::vector<std::any>& parameters) { return std::any(); }
		virtual std::size_t getNumberOfArguments() const { return argumentTypes.size(); }
		inline virtual uintptr_t getFunctionAddress() const {return 0;}

		const std::vector<CatGenericType>& getArgumentTypes() const {return argumentTypes;}
		const CatGenericType& getReturnType() const {return returnType;}

		template<typename ArgumentT>
		inline void addParameterTypeInfo()
		{
			argumentTypes.push_back(TypeTraits<typename RemoveConst<ArgumentT>::type >::toGenericType());
		}

		const CatGenericType& getArgumentType(std::size_t argumentIndex) const;


		// Inherited via FunctionSignature
		virtual const std::string& getLowerCaseFunctionName() const override;
		virtual int getNumParameters() const override;
		virtual const CatGenericType& getParameterType(int index) const override;

	private:
		std::string memberFunctionName;
		std::string lowerCaseFunctionName;
		CatGenericType returnType;
		MemberVisibility visibility;

		std::vector<CatGenericType> argumentTypes;

	};


	template <typename ReturnT, class ... TFunctionArguments>
	class StaticFunctionInfoWithArgs: public StaticFunctionInfo
	{
	public:
		StaticFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT (*function)(TFunctionArguments...)):
			StaticFunctionInfo(memberFunctionName, TypeTraits<std::remove_cv_t<ReturnT>>::toGenericType()),
			function(function)
		{
			//Trick to call a function per variadic template item
			//https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion
			//This gets the type info per parameter type
			int dummy[] = { 0, ( (void) addParameterTypeInfo<TFunctionArguments>(), 0) ... };
		}

	inline virtual std::any call(CatRuntimeContext* runtimeContext, const std::vector<std::any>& parameters) override final
	{ 
		//Generate a list of indices (statically) so the parameters list can be indices by the variadic template parameter index.
		return callWithIndexed(parameters, BuildIndices<sizeof...(TFunctionArguments)>{});
	}


	template<std::size_t... Is>
	std::any callWithIndexed(const std::vector<std::any>& parameters, Indices<Is...>)
	{
		if constexpr (std::is_same<void, ReturnT>::value)
		{
			return (*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...);
		}
		else
		{
			return TypeTraits<ReturnT>::getValue((*function)(TypeConversionCast::convertCast<TFunctionArguments, typename TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValueType >(TypeTraits<typename RemoveConst<TFunctionArguments>::type>::getValue(parameters[Is]))...));
		}
	}


	virtual std::size_t getNumberOfArguments() const override final
	{ 
		return sizeof...(TFunctionArguments);
	}


	inline virtual uintptr_t getFunctionAddress() const override final
	{
		uintptr_t pointer = 0;
		memcpy(&pointer, &function, sizeof(uintptr_t));
		return pointer;
	}


	private:
		ReturnT (*function)(TFunctionArguments...);
	};
}