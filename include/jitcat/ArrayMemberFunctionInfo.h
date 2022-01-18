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
	class ArrayTypeInfo;

	struct ArrayMemberFunctionInfo: public MemberFunctionInfo
	{
	public:
		enum class Operation
		{
			Index,
			Size,
			Init,
			Destroy,
			Resize
		};

		static const char* toString(Operation operation);

		ArrayMemberFunctionInfo(Operation operation, ArrayTypeInfo* arrayTypeInfo);

		virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const override final;

		virtual MemberFunctionCallData getFunctionAddress(FunctionType functionType) const override final;

	private:
		void doInit(std::any& base, const std::vector<std::any>& parameters) const;
		void doDestroy(std::any& base, const std::vector<std::any>& parameters) const;
		void doResize(std::any& base, const std::vector<std::any>& parameters) const;
		
		static llvm::Value* generateArraySizePtr(LLVM::LLVMCompileTimeContext* context, llvm::Value* arrayPointer);
		static llvm::Value* generateGetArraySize(LLVM::LLVMCompileTimeContext* context, llvm::Value* arrayPointer);
		
		static void generateInitEmptyArray(LLVM::LLVMCompileTimeContext* context, llvm::Value* arrayPointer, llvm::Value* arraySizePointer);
		static void generateAllocateArray(LLVM::LLVMCompileTimeContext* context, ArrayTypeInfo* arrayType, llvm::Value* arrayPointer, llvm::Value* arraySizePointer, llvm::Value* arraySizeElements,
										  llvm::Value*& arrayData, llvm::Value*& arraySizeBytes, llvm::Value*& arrayItemSizeBytes);
		static void generateConstructArray(LLVM::LLVMCompileTimeContext* context, ArrayTypeInfo* arrayType, llvm::Value* arrayData, llvm::Value* arraySizeBytes, llvm::Value* arrayItemSizeBytes);
		static void generateDestroyArray(LLVM::LLVMCompileTimeContext* context, ArrayTypeInfo* arrayType, llvm::Value* arrayData, llvm::Value* arraySizeElements);
		static void generateFreeArray(LLVM::LLVMCompileTimeContext* context, llvm::Value* arrayPointer, llvm::Value* arrayData, llvm::Value* arraySizePointer);
		static void generateMoveArrayElements(LLVM::LLVMCompileTimeContext* context, ArrayTypeInfo* arrayType, llvm::Value* targetArrayData, llvm::Value* sourceArrayData, llvm::Value* numItemsToMove, llvm::Value* itemSize);

		void createIndexGeneratorFunction();
		void createSizeGeneratorFunction();
		void createInitGeneratorFunction();
		void createDestroyGeneratorFunction();
		void createResizeGeneratorFunction();

		llvm::Type* getIndexReturnType(LLVM::LLVMCodeGeneratorHelper* codeGeneratorHelper) const;

	private:
		Operation operation;
		ArrayTypeInfo* arrayTypeInfo;

		//A generator function for generating inline LLVM IR for the array member function
		std::unique_ptr<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>> inlineFunctionGenerator;
	};
}