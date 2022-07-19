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

namespace llvm
{
	class Value;
}

namespace jitcat::LLVM
{
	struct LLVMCompileTimeContext;
	class LLVMCodeGeneratorHelper;
}

namespace jitcat::Reflection
{
	class VectorTypeInfo;

	struct VectorMemberFunctionInfo: public MemberFunctionInfo
	{
	public:
		enum class Operation
		{
			Index,
			Init,
			Destroy
		};

		static const char* toString(Operation operation);

		VectorMemberFunctionInfo(Operation operation, VectorTypeInfo* vectorTypeInfo);

		virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const override final;

		virtual MemberFunctionCallData getFunctionAddress(FunctionType functionType) const override final;

	private:
		std::any doIndex(std::any& base, const std::vector<std::any>& parameters) const;
		void doInit(std::any& base, const std::vector<std::any>& parameters) const;
		
		static void generateConstructVector(LLVM::LLVMCompileTimeContext* context, VectorTypeInfo* vectorType, llvm::Value* vectorData, llvm::Value* initialData);
		static void generateMoveArrayElements(LLVM::LLVMCompileTimeContext* context, VectorTypeInfo* vectorType, llvm::Value* targetVectorData, llvm::Value* sourceVectorData);

		void createIndexGeneratorFunction();
		void createInitGeneratorFunction();
		void createDestroyGeneratorFunction();

		llvm::Type* getIndexReturnType(LLVM::LLVMCodeGeneratorHelper* codeGeneratorHelper) const;
		
		llvm::ArrayType* getLLVMVectorType(LLVM::LLVMCodeGeneratorHelper* codeGeneratorHelper) const;

	private:
		Operation operation;
		VectorTypeInfo* vectorTypeInfo;

		//A generator function for generating inline LLVM IR for the vector member function
		std::unique_ptr<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>> inlineFunctionGenerator;
	};
}