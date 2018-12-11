#pragma once

class CatRuntimeContext;
class LLVMCodeGeneratorHelper;
struct LLVMCompileTimeContext;
class Reflectable;
#include "LLVMForwardDeclares.h"
#include <string>


class LLVMCatIntrinsics
{
public:
	LLVMCatIntrinsics(llvm::LLVMContext* context, LLVMCodeGeneratorHelper* helper, llvm::Module* module);

	static Reflectable* getThisPointerFromContext(CatRuntimeContext* context);
	static Reflectable* getCustomThisPointerFromContext(CatRuntimeContext* context);
	llvm::Value* callGetThisPointer(llvm::Value* catRunTimeContext);
	llvm::Value* callGetCustomThisPointer(llvm::Value* catRunTimeContext);

	static bool stringEquals(const std::string& left, const std::string& right);
	static bool stringNotEquals(const std::string& left, const std::string& right);
	static std::string stringAppend(const std::string& left, const std::string& right);
	static std::string floatToString(float number);
	static std::string intToString(int number);
	static void stringCopy(std::string* destination, const std::string& string);
	static void stringDestruct(std::string* target);

	llvm::Value* callStringDestruct(llvm::Value* target);
	llvm::Value* callStringEquals(llvm::Value* left, llvm::Value* right);
	llvm::Value* callStringNotEquals(llvm::Value* left, llvm::Value* right);
	llvm::Value* callStringAppend(llvm::Value* left, llvm::Value* right, LLVMCompileTimeContext* context);
	llvm::Value* callFloatToString(llvm::Value* number, LLVMCompileTimeContext* context);
	llvm::Value* callIntToString(llvm::Value* number, LLVMCompileTimeContext* context);
	llvm::Value* callStringCopy(llvm::Value* destinationMemory, llvm::Value* string);

private:
	llvm::LLVMContext* context;
	LLVMCodeGeneratorHelper* helper;

	llvm::FunctionType* stringComparisonFunctionType;
	llvm::FunctionType* stringAppendFunctionType;
	llvm::FunctionType* stringDestructFunctionType;
	llvm::FunctionType* floatToStringFunctionType;
	llvm::FunctionType* intToStringFunctionType;
	llvm::FunctionType* stringCopyFunctionType;
};