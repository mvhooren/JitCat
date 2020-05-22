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
		
		MemberVisibility getVisibility() const;

		template<typename ArgumentT>
		inline void addParameterTypeInfo();

		const CatGenericType& getArgumentType(std::size_t argumentIndex) const;


		// Inherited via FunctionSignature
		virtual const std::string& getLowerCaseFunctionName() const override;
		virtual int getNumParameters() const override;
		virtual const CatGenericType& getParameterType(int index) const override;

		const std::string& getNormalFunctionName() const;

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
		inline StaticFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(*function)(TFunctionArguments...));

		inline virtual std::any call(CatRuntimeContext* runtimeContext, const std::vector<std::any>& parameters) override final;

		template<std::size_t... Is>
		std::any callWithIndexed(const std::vector<std::any>& parameters, Indices<Is...>);

		virtual std::size_t getNumberOfArguments() const override final;
		inline virtual uintptr_t getFunctionAddress() const override final;

	private:
		ReturnT (*function)(TFunctionArguments...);
	};



}


#include "jitcat/StaticMemberFunctionInfoHeaderImplementation.h"