/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/BuildIndicesHelper.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/MemberVisibility.h"

#include <functional>
#include <string>
#include <vector>


namespace jitcat
{
	class CatRuntimeContext;
	namespace LLVM
	{
		struct LLVMCompileTimeContext;
	}
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
		InlineFunctionGenerator,
		Unknown
	};

	struct MemberFunctionCallData
	{
		MemberFunctionCallData(): functionAddress(0), functionInfoStructAddress(0), callType(MemberFunctionCallType::Unknown), inlineFunctionGenerator(nullptr), linkDylib(false) {}
		MemberFunctionCallData(uintptr_t functionAddress, uintptr_t functionInfoStructAddress, 
							   const std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>* inlineFunctionGenerator,
							   MemberFunctionCallType callType, bool linkDylib): 
			functionAddress(functionAddress), 
			functionInfoStructAddress(functionInfoStructAddress),
			inlineFunctionGenerator(inlineFunctionGenerator),
			callType(callType),
			linkDylib(linkDylib)
		{}
		const uintptr_t functionAddress;
		const uintptr_t functionInfoStructAddress;
		const std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>* inlineFunctionGenerator;
		const MemberFunctionCallType callType;
		const bool linkDylib;
	};


	struct MemberFunctionInfo: public FunctionSignature
	{
		MemberFunctionInfo(const std::string& memberFunctionName, const CatGenericType& returnType);
		virtual ~MemberFunctionInfo() {}

		const CatGenericType& getReturnType() const;
		void setReturnType(const CatGenericType& newReturnType);

		MemberVisibility getVisibility() const;
		void setVisibility(MemberVisibility newVisibility);

		const std::string& getMemberFunctionName() const;

		const std::vector<CatGenericType>& getArgumentTypes() const;
		const CatGenericType& getArgumentType(std::size_t argumentIndex) const;
		DeferredMemberFunctionInfo* toDeferredMemberFunction(TypeMemberInfo* baseMember) const;


		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const { return std::any(); }
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

		void addParameterType(const CatGenericType& type);


	protected:
		const std::string memberFunctionName;
		const std::string lowerCaseMemberFunctionName;
		CatGenericType returnType;
		MemberVisibility visibility;

		std::vector<CatGenericType> argumentTypes;
	};

	struct DeferredMemberFunctionInfo : public MemberFunctionInfo
	{
		DeferredMemberFunctionInfo(TypeMemberInfo* baseMember, const MemberFunctionInfo* deferredFunction);

		const TypeMemberInfo* getBaseMember() const;
		TypeMemberInfo* getBaseMember();

		const MemberFunctionInfo* getDeferredFunction() const;

		virtual ~DeferredMemberFunctionInfo();
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const override final;
		virtual std::size_t getNumberOfArguments() const override final;
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		inline virtual bool isDeferredFunctionCall() override final;
		virtual std::string getMangledName() const override final;

	private:
		TypeMemberInfo* baseMember;
		const MemberFunctionInfo* deferredFunction;

	};


	template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
	struct MemberFunctionInfoWithArgs: public MemberFunctionInfo
	{
		inline MemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(ClassT::* function)(TFunctionArguments...));
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const override final;

		template<std::size_t... Is>
		inline std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const;

		inline virtual std::size_t getNumberOfArguments() const override final;
		static inline ReturnT staticExecute(ClassT* base, MemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args);
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		virtual std::string getMangledName() const override final;

	private:
		ReturnT (ClassT::*function)(TFunctionArguments...);
	};


	template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
	struct ConstMemberFunctionInfoWithArgs: public MemberFunctionInfo
	{
		inline ConstMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(ClassT::* function)(TFunctionArguments...) const);
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const override final;

		template<std::size_t... Is>
		inline std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const;

		inline virtual std::size_t getNumberOfArguments() const override final;
		static inline ReturnT staticExecute(ClassT* base, ConstMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args);
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		virtual std::string getMangledName() const override final;

	private:
		ReturnT (ClassT::*function)(TFunctionArguments...) const;
	};


	template <typename ClassT, typename ReturnT, class ... TFunctionArguments>
	struct PseudoMemberFunctionInfoWithArgs: public MemberFunctionInfo
	{
		inline PseudoMemberFunctionInfoWithArgs(const std::string& memberFunctionName, ReturnT(*function)(ClassT*, TFunctionArguments...));
		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const override final;

		template<std::size_t... Is>
		inline std::any callWithIndexed(const std::vector<std::any>& parameters, std::any& base, Indices<Is...>) const;

		virtual inline std::size_t getNumberOfArguments() const override final;
		static inline ReturnT staticExecute(ClassT* base, PseudoMemberFunctionInfoWithArgs<ClassT, ReturnT, TFunctionArguments...>* functionInfo, TFunctionArguments... args);
		inline virtual MemberFunctionCallData getFunctionAddress() const override final;
		virtual std::string getMangledName() const override final;

	private:
		ReturnT (*function)(ClassT*, TFunctionArguments...);
	};


} //End namespace jitcat::Reflection

#include "jitcat/MemberFunctionInfoHeaderImplementation.h"
