/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatRuntimeContext;
}
#include "jitcat/CatGenericType.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/LLVMTypes.h"

#include <functional>
#include <memory>
#include <vector>


namespace jitcat::LLVM
{
	struct LLVMCompileTimeContext;


	class LLVMCodeGeneratorHelper
	{
	public:
		LLVMCodeGeneratorHelper(llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* builder, llvm::Module* module);

		template<typename T, typename ... Args>
		llvm::Value* createCall(LLVMCompileTimeContext* context, T (*functionPointer)(Args ...), const std::vector<llvm::Value*>& arguments, const std::string& name);
		llvm::Value* createCall(llvm::FunctionType* functionType, uintptr_t functionAddress, const std::vector<llvm::Value*>& arguments, const std::string& functionName);
		llvm::Value* createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, llvm::Type* resultType, LLVMCompileTimeContext* context); 
		llvm::Value* createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, llvm::PointerType* resultType, LLVMCompileTimeContext* context); 
		llvm::Value* createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNull, LLVMCompileTimeContext* context); 

		llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& parameterType, llvm::Value* argument, LLVMCompileTimeContext* context);
		llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& overload1Type, llvm::Value* argument1, llvm::Value* argument2, LLVMCompileTimeContext* context);
		llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& overload1Type, const CatGenericType& overload2Type, llvm::Value* argument1, llvm::Value* argument2, LLVMCompileTimeContext* context);
		llvm::Type* toLLVMType(const CatGenericType& type);
		llvm::PointerType* toLLVMPtrType(const CatGenericType& type);

		void writeToPointer(llvm::Value* lValue, llvm::Value* rValue);

		llvm::Value* convertType(llvm::Value* valueToConvert, llvm::Type* type, LLVMCompileTimeContext* context);
		llvm::Value* convertToPointer(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type = LLVMTypes::pointerType);
		llvm::Value* convertToPointer(llvm::Constant* addressConstant, const std::string& name, llvm::PointerType* type = LLVMTypes::pointerType);
		llvm::Value* convertToIntPtr(llvm::Value* llvmPointer, const std::string& name);

		static bool isPointer(llvm::Type* type);
		static bool isStringPointer(llvm::Type* type);
		static bool isIntPtr(llvm::Type* type);
		static bool isInt(llvm::Type* type);

		static bool isPointer(llvm::Value* value);
		static bool isStringPointer(llvm::Value* value);
		static bool isIntPtr(llvm::Value* value);
		static bool isInt(llvm::Value* value);

		llvm::Value* loadBasicType(llvm::Type* type, llvm::Value* addressValue, const std::string& name);
		llvm::Value* loadPointerAtAddress(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type = LLVMTypes::pointerType);
		llvm::Value* createAdd(llvm::Value* value1, llvm::Value* value2, const std::string& name);
		llvm::Value* createAdd(llvm::Value* value1, llvm::Constant* value2, const std::string& name);

		llvm::Constant* createZeroInitialisedConstant(llvm::Type* type);
		llvm::Constant* createIntPtrConstant(unsigned long long constant, const std::string& name);
		llvm::Constant* createCharConstant(char constant);
		llvm::Constant* createUCharConstant(unsigned char constant);
		llvm::Constant* createConstant(int constant);
		llvm::Constant* createConstant(float constant);
		llvm::Constant* createConstant(bool constant);
		llvm::Constant* createNullPtrConstant(llvm::PointerType* pointerType);
		llvm::Value* createPtrConstant(unsigned long long address, const std::string& name, llvm::PointerType* pointerType = LLVMTypes::pointerType);
		llvm::Constant* createZeroInitialisedArrayConstant(llvm::ArrayType* arrayType);
		llvm::Value* createEmptyStringPtrConstant();


		void setCurrentModule(llvm::Module* module);

		llvm::Value* createObjectAllocA(LLVMCompileTimeContext* context, const std::string& name, const CatGenericType& objectType, bool generateDestructorCall);
		llvm::Value* createStringAllocA(LLVMCompileTimeContext* context, const std::string& name, bool generateDestructorCall);
		void generateBlockDestructors(LLVMCompileTimeContext* context);

		llvm::LLVMContext& getContext();
		llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* getBuilder();

		static llvm::FunctionType* createFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& argumentTypes);


	private:
		llvm::Value* generateCall(LLVMCompileTimeContext* context, uintptr_t functionAddress, llvm::FunctionType* functionType, const std::vector<llvm::Value*>& arguments, bool isStructRet, const std::string& name);
		llvm::Value* createZeroStringPtrConstant();
		llvm::Value* createOneStringPtrConstant();

	private:
		llvm::LLVMContext& llvmContext;
		llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* builder;
		llvm::Module* currentModule;

		static const std::string emptyString;
		static const std::string oneString;
		static const std::string zeroString;
	};



	template<typename TReturnType, typename ...TFunctionArguments>
	class LLVMFunctionTypeGenerator
	{
		LLVMFunctionTypeGenerator();
		~LLVMFunctionTypeGenerator() = delete;
	public:
		static llvm::FunctionType* getType(bool& isStructRet)
		{
			llvm::Type* returnType = LLVMTypes::getLLVMType<TReturnType>();
			std::vector<llvm::Type*> argumentTypes;
			if (LLVMCodeGeneratorHelper::isStringPointer(returnType))
			{
				isStructRet = true;
				argumentTypes.push_back(returnType);
				returnType = LLVMTypes::voidType;
			}
			int dummy[] = { 0, ( (void) addArgumentType<TFunctionArguments>(argumentTypes), 0) ... };
			return LLVMCodeGeneratorHelper::createFunctionType(returnType, argumentTypes);
		}

		template<typename TArgumentType>
		static void addArgumentType(std::vector<llvm::Type*>& argumentTypes)
		{
			argumentTypes.push_back(LLVMTypes::getLLVMType<TArgumentType>());
		}
	};


	template<typename T, typename ...Args>
	inline llvm::Value* LLVMCodeGeneratorHelper::createCall(LLVMCompileTimeContext* context, T(*functionPointer)(Args...), const std::vector<llvm::Value*>& arguments, const std::string& name)
	{
		bool isStructRet = false;
		llvm::FunctionType* functionType = LLVMFunctionTypeGenerator<T, Args...>::getType(isStructRet);
		return generateCall(context, reinterpret_cast<uintptr_t>(functionPointer), functionType, arguments, isStructRet, name);
	}

} //End namespace jitcat::LLVM