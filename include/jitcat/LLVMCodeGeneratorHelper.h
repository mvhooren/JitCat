#pragma once

class CatRuntimeContext;
class Reflectable;
struct LLVMCompileTimeContext;
#include "CatType.h"
#include "LLVMCatIntrinsics.h"
#include "LLVMForwardDeclares.h"
#include "LLVMTypes.h"

#include <memory>
#include <vector>


class LLVMCodeGeneratorHelper
{
public:
	LLVMCodeGeneratorHelper(llvm::IRBuilder<>* builder, llvm::Module* module);

	llvm::Value* callFunction(llvm::FunctionType* functionType, uintptr_t functionAddress, const std::vector<llvm::Value*>& arguments, const std::string& functionName);

	llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, CatType parameterType, llvm::Value* argument, LLVMCompileTimeContext* context);
	llvm::Value* callIntrinsic(llvm::Intrinsic::ID intrinsic, CatType parameter1Type, CatType parameter2Type, llvm::Value* argument1, llvm::Value* argument2, LLVMCompileTimeContext* context);
	llvm::Type* toLLVMType(CatType type);

	llvm::Value* convertType(llvm::Value* valueToConvert, llvm::Type* type, LLVMCompileTimeContext* context);
	llvm::Value* convertToPointer(llvm::Value* addressValue, const std::string& name, llvm::Type* type = LLVMTypes::pointerType);
	llvm::Value* convertToIntPtr(llvm::Value* llvmPointer, const std::string& name);
	bool isPointer(llvm::Value* value) const;
	bool isStringPointer(llvm::Value* value) const;
	bool isIntPtr(llvm::Value* value) const;

	llvm::Value* loadBasicType(llvm::Type* type, llvm::Value* addressValue, const std::string& name);
	llvm::Value* loadPointerAtAddress(llvm::Value* addressValue, const std::string& name, llvm::Type* type = LLVMTypes::pointerType);
	llvm::Value* createAdd(llvm::Value* value1, llvm::Value* value2, const std::string& name);

	llvm::Value* createIntPtrConstant(unsigned long long constant, const std::string& name);
	llvm::Value* createConstant(int constant);
	llvm::Value* createConstant(float constant);
	llvm::Value* createConstant(bool constant);

	void setCurrentModule(llvm::Module* module);

	llvm::Value* createStringAllocA(LLVMCompileTimeContext* context, const std::string& name);
	void generateBlockDestructors(LLVMCompileTimeContext* context);

	llvm::LLVMContext& getContext();
	llvm::IRBuilder<>* getBuilder();

public:
	std::unique_ptr<LLVMCatIntrinsics> intrinsics;

private:
	llvm::LLVMContext& llvmContext;
	llvm::IRBuilder<>* builder;
	llvm::Module* currentModule;
};