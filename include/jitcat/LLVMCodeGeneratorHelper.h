/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatGenericType;
	class CatRuntimeContext;
	namespace AST
	{
		class CatTypedExpression;
	}
	namespace Reflection
	{
		class CustomTypeInfo;
		class StaticFunctionInfo;
		struct MemberFunctionInfo;
	}
}
#include "jitcat/CatScopeID.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/LLVMTypes.h"


#include <functional>
#include <memory>
#include <vector>


namespace jitcat::LLVM
{
	struct LLVMCompileTimeContext;
	class LLVMCodeGenerator;

	class LLVMCodeGeneratorHelper
	{
	public:
		LLVMCodeGeneratorHelper(LLVMCodeGenerator* codeGenerator);

		template<typename ReturnT, typename ... Args>
		llvm::Value* createIntrinsicCall(LLVMCompileTimeContext* context, ReturnT (*functionPointer)(Args ...), const std::vector<llvm::Value*>& arguments, const std::string& name);
		llvm::Value* createCall(llvm::FunctionType* functionType, const std::vector<llvm::Value*>& arguments, bool isThisCall, const std::string& mangledFunctionName, const std::string& shortFunctionName);
		
		llvm::Value* createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull,
										   llvm::Type* resultType, LLVMCompileTimeContext* context); 
		llvm::Value* createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, 
										   llvm::PointerType* resultType, LLVMCompileTimeContext* context); 
		llvm::Value* createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, 
										   std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNull, LLVMCompileTimeContext* context); 
		
		llvm::Value* createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull,
												   llvm::Type* resultType, LLVMCompileTimeContext* context); 
		llvm::Value* createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, 
												   llvm::PointerType* resultType, LLVMCompileTimeContext* context); 
		llvm::Value* createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, 
												   std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNull, LLVMCompileTimeContext* context); 

		llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& parameterType, 
								   llvm::Value* argument, const CatGenericType& argumentType, 
								   LLVMCompileTimeContext* context);
		llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& overload1Type, 
								   llvm::Value* argument1, const CatGenericType& argument1Type, 
								   llvm::Value* argument2, const CatGenericType& argument2Type, 
								   LLVMCompileTimeContext* context);
		llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& overload1Type, const CatGenericType& overload2Type, 
								   llvm::Value* argument1, const CatGenericType& argument1Type, 
								    llvm::Value* argument2, const CatGenericType& argument2Type, LLVMCompileTimeContext* context);
		llvm::Type* toLLVMType(const CatGenericType& type);
		llvm::PointerType* toLLVMPtrType(const CatGenericType& type);

		void writeToPointer(llvm::Value* lValue, llvm::Value* rValue);

		llvm::Value* convertType(llvm::Value* valueToConvert, const CatGenericType& fromType, const CatGenericType& toType, LLVMCompileTimeContext* context);
		llvm::Value* convertType(llvm::Value* valueToConvert, bool valueIsSigned, llvm::Type* toType, bool toIsSigned, LLVMCompileTimeContext* context);
	private:
		llvm::Value* convertToString(llvm::Value* valueToConvert, const CatGenericType& fromType, LLVMCompileTimeContext* context);
	public:
		llvm::Value* convertToPointer(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type = LLVMTypes::pointerType);
		llvm::Value* convertToPointer(llvm::Constant* addressConstant, const std::string& name, llvm::PointerType* type = LLVMTypes::pointerType);
		llvm::Value* convertToIntPtr(llvm::Value* llvmPointer, const std::string& name);

		static bool isPointer(llvm::Type* type);
		static bool isIntPtr(llvm::Type* type);
		static bool isInt(llvm::Type* type);

		static bool isPointer(llvm::Value* value);
		static bool isIntPtr(llvm::Value* value);
		static bool isInt(llvm::Value* value);

		llvm::Value* loadBasicType(llvm::Type* type, llvm::Value* addressValue, const std::string& name);
		llvm::Value* loadBasicType(llvm::Type* type, llvm::Constant* addressValue, const std::string& name);
		llvm::Value* loadPointerAtAddress(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type = LLVMTypes::pointerType);
		llvm::Value* loadPointerAtAddress(llvm::Constant* addressValue, const std::string& name, llvm::PointerType* type = LLVMTypes::pointerType);
		llvm::Value* createAdd(llvm::Value* value1, llvm::Value* value2, const std::string& name);
		llvm::Value* createAdd(llvm::Value* value1, llvm::Constant* value2, const std::string& name);

		llvm::Constant* createZeroInitialisedConstant(llvm::Type* type);
		llvm::Constant* createIntPtrConstant(unsigned long long constant, const std::string& name);
		llvm::Constant* createCharConstant(char constant);
		llvm::Constant* createUCharConstant(unsigned char constant);
		llvm::Constant* createConstant(char constant);
		llvm::Constant* createConstant(unsigned char constant);
		llvm::Constant* createConstant(int constant);
		llvm::Constant* createConstant(unsigned int constant);
		llvm::Constant* createConstant(int64_t constant);
		llvm::Constant* createConstant(uint64_t constant);
		llvm::Constant* createConstant(double constant);
		llvm::Constant* createConstant(float constant);
		llvm::Constant* createConstant(bool constant);
		llvm::Constant* createNullPtrConstant(llvm::PointerType* pointerType);
		llvm::Constant* createZeroTerminatedStringConstant(const std::string& value);
		llvm::GlobalVariable* createGlobalPointerSymbol(const std::string& name);
		llvm::Value* createPtrConstant(unsigned long long address, const std::string& name, llvm::PointerType* pointerType = LLVMTypes::pointerType);
		llvm::Constant* createZeroInitialisedArrayConstant(llvm::ArrayType* arrayType);
		llvm::Value* constantToValue(llvm::Constant* constant) const;

		llvm::Value* createObjectAllocA(LLVMCompileTimeContext* context, const std::string& name, const CatGenericType& objectType, bool generateDestructorCall);
		void generateBlockDestructors(LLVMCompileTimeContext* context);

		llvm::LLVMContext& getContext();
		llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* getBuilder();

		llvm::Value* generateFunctionCallReturnValueAllocation(const CatGenericType& returnType, const std::string& functionName, LLVMCompileTimeContext* context);
		void generateFunctionCallArgumentEvalatuation(const std::vector<const jitcat::AST::CatTypedExpression*>& arguments, 
													  const std::vector<CatGenericType>& expectedArgumentTypes, 
													  std::vector<llvm::Value*>& generatedArguments,
													  std::vector<llvm::Type*>& generatedArgumentTypes,
													  LLVMCodeGenerator* generator, LLVMCompileTimeContext* context);

		llvm::Value* generateStaticFunctionCall(const jitcat::CatGenericType& returnType, 
												const std::vector<llvm::Value*>& argumentList, 
												const std::vector<llvm::Type*>& argumentTypes, 
												LLVMCompileTimeContext* context,
												const std::string& mangledFunctionName, 
												const std::string& shortFunctionName,
												llvm::Value* returnedObjectAllocation);

		llvm::Value* generateMemberFunctionCall(jitcat::Reflection::MemberFunctionInfo* memberFunction, const jitcat::AST::CatTypedExpression* base, 
											    const std::vector<const jitcat::AST::CatTypedExpression*>& arguments, 
												LLVMCompileTimeContext* context);

		//Generates a simple loop
		void generateLoop(LLVMCompileTimeContext* context,
						  llvm::Value* iteratorBeginValue,
						  llvm::Value* iteratorStepValue,
						  llvm::Value* iteratorEndValue,
						  const std::function<void (LLVMCompileTimeContext*, llvm::Value*)>& generateLoopBody);
		
		
		void defineWeakSymbol(intptr_t functionAddress, const std::string& mangledFunctionName);

	private:
		llvm::Value* convertIndirection(llvm::Value* value, llvm::Type* expectedType);
		llvm::Value* copyConstructIfValueType(llvm::Value* value, const CatGenericType& type, LLVMCompileTimeContext* context, const std::string& valueName);
		llvm::Value* generateIntrinsicCall(jitcat::Reflection::StaticFunctionInfo* functionInfo, std::vector<llvm::Value*>& arguments, LLVMCompileTimeContext* context);

		llvm::Value* createZeroStringPtrConstant();
		llvm::Value* createOneStringPtrConstant();

	private:
		LLVMCodeGenerator* codeGenerator;
		llvm::LLVMContext& llvmContext;

		static const std::string emptyString;
		static const std::string oneString;
		static const std::string zeroString;
	};

}

#include "jitcat/StaticMemberFunctionInfo.h"


namespace jitcat::LLVM
{

	template<typename ReturnT, typename ...Args>
	inline llvm::Value* LLVMCodeGeneratorHelper::createIntrinsicCall(LLVMCompileTimeContext* context, ReturnT (*functionPointer)(Args...), const std::vector<llvm::Value*>& arguments, const std::string& name)
	{
		Reflection::StaticFunctionInfoWithArgs<ReturnT, Args...> functionInfo(name, nullptr, functionPointer);
		std::vector<llvm::Value*> argumentsCopy = arguments;
		return generateIntrinsicCall(&functionInfo, argumentsCopy, context);
	}

} //End namespace jitcat::LLVM