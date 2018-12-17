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
	LLVMCodeGeneratorHelper(llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* builder, llvm::Module* module);

	template<typename T, typename ... Args>
	llvm::Value* createCall(LLVMCompileTimeContext* context, T (*functionPointer)(Args ...), const std::vector<llvm::Value*>& arguments, const std::string& name);
	llvm::Value* createCall(llvm::FunctionType* functionType, uintptr_t functionAddress, const std::vector<llvm::Value*>& arguments, const std::string& functionName);

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
	llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* getBuilder();

	static llvm::FunctionType* createFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& argumentTypes);

private:
	llvm::Value* generateCall(LLVMCompileTimeContext* context, uintptr_t functionAddress, llvm::FunctionType* functionType, const std::vector<llvm::Value*>& arguments, bool isStructRet, const std::string& name);

private:
	llvm::LLVMContext& llvmContext;
	llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* builder;
	llvm::Module* currentModule;
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
		if (returnType == LLVMTypes::stringPtrType)
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