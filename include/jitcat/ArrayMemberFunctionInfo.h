/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/MemberFunctionInfo.h"

#include <functional>
#include <memory>
#include <vector>


namespace jitcat::LLVM
{
	struct LLVMCompileTimeContext;
	class LLVMCodeGeneratorHelper;
}

namespace jitcat::Reflection
{
	class ArrayTypeInfo;

	struct ArrayMemberFunctionInfo: public MemberFunctionInfo
	{
	public:
		enum class Operation
		{
			Index,
			Size,
			Init,
			Destroy
		};

		static const char* toString(Operation operation);

		ArrayMemberFunctionInfo(Operation operation, ArrayTypeInfo* arrayTypeInfo);

		virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const override final;

		virtual MemberFunctionCallData getFunctionAddress() const override final;

	private:
		void createIndexGeneratorFunction();
		void createSizeGeneratorFunction();
		void createInitGeneratorFunction();
		void createDestroyGeneratorFunction();

		llvm::Type* getIndexReturnType(LLVM::LLVMCodeGeneratorHelper* codeGeneratorHelper) const;

	private:
		Operation operation;
		ArrayTypeInfo* arrayTypeInfo;

		//A generator function for generating inline LLVM IR for the array member function
		std::unique_ptr<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>> inlineFunctionGenerator;
	};
}