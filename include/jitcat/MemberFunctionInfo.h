/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
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
		MemberFunctionCallData(): functionAddress(0), functionInfoStructAddress(0), callType(MemberFunctionCallType::Unknown), generateSymbol(false) {}
		MemberFunctionCallData(uintptr_t functionAddress, uintptr_t functionInfoStructAddress, MemberFunctionCallType callType, bool generateSymbol): 
			functionAddress(functionAddress), 
			functionInfoStructAddress(functionInfoStructAddress), 
			callType(callType),
			generateSymbol(generateSymbol)
		{}
		uintptr_t functionAddress;
		uintptr_t functionInfoStructAddress;
		MemberFunctionCallType callType;
		bool generateSymbol;
	};


	struct MemberFunctionInfo: public FunctionSignature
	{
		MemberFunctionInfo(const std::string& memberFunctionName, const CatGenericType& returnType);;
		virtual ~MemberFunctionInfo() {}
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) { return std::any(); }
		virtual std::size_t getNumberOfArguments() const { return argumentTypes.size(); }
		inline virtual MemberFunctionCallData getFunctionAddress() const {return MemberFunctionCallData();}
		inline virtual bool isDeferredFunctionCall() {return false;}
		// Inherited via FunctionSignature
		virtual const std::string& getLowerCaseFunctionName() const override final;
		virtual int getNumParameters() const override final; 
		virtual const CatGenericType& getParameterType(int index) const override final;
		virtual std::string getMangledName() const;
		template<typename ArgumentT>
		inline void addParameterTypeInfo();

		CatGenericType getArgumentType(std::size_t argumentIndex) const;
		DeferredMemberFunctionInfo* toDeferredMemberFunction(TypeMemberInfo* baseMember);

		std::string memberFunctionName;
		std::string lowerCaseMemberFunctionName;
		CatGenericType returnType;
		MemberVisibility visibility;

		std::vector<CatGenericType> argumentTypes;
	};

	struct DeferredMemberFunctionInfo : public MemberFunctionInfo
	{
		DeferredMemberFunctionInfo(TypeMemberInfo* baseMember, MemberFunctionInfo* deferredFunction);

		virtual ~DeferredMemberFunctionInfo();
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final;
		virtual std::size_t getNumberOfArguments() const override final;
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		inline virtual bool isDeferredFunctionCall() override final;
		virtual std::string getMangledName() const override final;
		TypeMemberInfo* baseMember;
		MemberFunctionInfo* deferredFunction;

	};


	template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
	struct MemberFunctionInfoWithArgs: public MemberFunctionInfo
	{
		inline MemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(ClassT::* function)(TFunctionArguments...));
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final;

		template<std::size_t... Is>
		inline std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>);

		inline virtual std::size_t getNumberOfArguments() const override final;
		static inline ReturnT staticExecute(ClassT* base, MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args);
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		virtual std::string getMangledName() const override final;

		ReturnT (ClassT::*function)(TFunctionArguments...);
	};


	template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
	struct ConstMemberFunctionInfoWithArgs: public MemberFunctionInfo
	{
		inline ConstMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(ClassT::* function)(TFunctionArguments...) const);
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final;

		template<std::size_t... Is>
		inline std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const;

		inline virtual std::size_t getNumberOfArguments() const override final;
		static inline ReturnT staticExecute(ClassT* base, ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args);
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		virtual std::string getMangledName() const override final;

		ReturnT (ClassT::*function)(TFunctionArguments...) const;
	};


	template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
	struct PseudoMemberFunctionInfoWithArgs: public MemberFunctionInfo
	{
		inline PseudoMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(*function)(ClassT*, TFunctionArguments...));
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final;

		template<std::size_t... Is>
		inline std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>);

		virtual inline std::size_t getNumberOfArguments() const override final;
		static inline ReturnT staticExecute(ClassT* base, PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args);
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		virtual std::string getMangledName() const override final;

		ReturnT (*function)(ClassT*, TFunctionArguments...);
	};


} //End namespace jitcat::Reflection

#include "jitcat/MemberFunctionInfoHeaderImplementation.h"
