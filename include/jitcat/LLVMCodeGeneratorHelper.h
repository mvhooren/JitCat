#pragma once

class CatRuntimeContext;
class Reflectable;
#include "CatType.h"

#include "LLVMForwardDeclares.h"
#include <vector>


class LLVMCodeGeneratorHelper
{
public:
	LLVMCodeGeneratorHelper(llvm::IRBuilder<>* builder);

	static Reflectable* getThisPointerFromContext(CatRuntimeContext* context);
	static Reflectable* getCustomThisPointerFromContext(CatRuntimeContext* context);

	llvm::Value* callGetThisPointer(llvm::Value* catRunTimeContext);
	llvm::Value* callGetCustomThisPointer(llvm::Value* catRunTimeContext);
	llvm::Value* callFunction(llvm::FunctionType* functionType, uintptr_t functionAddress, const std::vector<llvm::Value*>& arguments);

	llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, CatType parameterType, llvm::Value* argument);
	llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, CatType parameter1Type, CatType parameter2Type, llvm::Value* argument1, llvm::Value* argument2);
	llvm::Type* toLLVMType(CatType type);

	llvm::Value* convertType(llvm::Value* valueToConvert, llvm::Type* type);
	llvm::Value* convertToPointer(llvm::Value* addressValue);
	llvm::Value* convertToIntPtr(llvm::Value* llvmPointer);
	bool isPointer(llvm::Value* value) const;
	bool isIntPtr(llvm::Value* value) const;

	llvm::Value* loadBasicType(llvm::Type* type, llvm::Value* addressValue);
	llvm::Value* loadPointerAtAddress(llvm::Value* addressValue);
	llvm::Value* createAdd(llvm::Value* value1, llvm::Value* value2);

	llvm::Value* createIntPtrConstant(unsigned long long constant);
	llvm::Value* createConstant(int constant);
	llvm::Value* createConstant(float constant);
	llvm::Value* createConstant(bool constant);

	void setCurrentModule(llvm::Module* module);
	
	llvm::LLVMContext& getContext();
	llvm::IRBuilder<>* getBuilder();

private:
	llvm::LLVMContext& llvmContext;
	llvm::IRBuilder<>* builder;
	llvm::Module* currentModule;
};