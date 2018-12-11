#include "LLVMCatIntrinsics.h"
#include "CatRuntimeContext.h"
#include "LLVMCodeGeneratorHelper.h"
#include "LLVMCompileTimeContext.h"
#include "LLVMTypes.h"
#include "MemberReference.h"
#include "Tools.h"

//QQQ not portable, fix
#pragma warning(push, 0) 
#include <llvm\IR\IRBuilder.h>
#include <llvm\IR\Function.h>
#pragma warning(pop)


LLVMCatIntrinsics::LLVMCatIntrinsics(llvm::LLVMContext* context, LLVMCodeGeneratorHelper* helper, llvm::Module* module):
	context(context),
	helper(helper)
{
	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::stringPtrType, LLVMTypes::stringPtrType};
		stringComparisonFunctionType = llvm::FunctionType::get(LLVMTypes::boolType, parameters, false);
	}
	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::stringPtrType, LLVMTypes::stringPtrType, LLVMTypes::stringPtrType};
		stringAppendFunctionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);
	}
	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::stringPtrType, LLVMTypes::floatType};
		floatToStringFunctionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);
	}
	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::stringPtrType, LLVMTypes::intType};
		intToStringFunctionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);
	}
	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::stringPtrType, LLVMTypes::stringPtrType};
		stringCopyFunctionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);
	}
	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::stringPtrType};
		stringDestructFunctionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);
	}
}


Reflectable* LLVMCatIntrinsics::getThisPointerFromContext(CatRuntimeContext* context)
{
	return context->getThisReference().getPointer()->getParentObject();
}


Reflectable* LLVMCatIntrinsics::getCustomThisPointerFromContext(CatRuntimeContext* context)
{
	return context->getCustomThisReference().getPointer()->getParentObject();
}


llvm::Value* LLVMCatIntrinsics::callGetThisPointer(llvm::Value* catRunTimeContext)
{
	return helper->callFunction(LLVMTypes::functionRetPtrArgPtr, reinterpret_cast<uintptr_t>(&getThisPointerFromContext), {catRunTimeContext}, "getThisPointerFromContext");
}


llvm::Value* LLVMCatIntrinsics::callGetCustomThisPointer(llvm::Value* catRunTimeContext)
{
	return helper->callFunction(LLVMTypes::functionRetPtrArgPtr, reinterpret_cast<uintptr_t>(&getCustomThisPointerFromContext), {catRunTimeContext}, "callGetCustomThisPointer");
}


bool LLVMCatIntrinsics::stringEquals(const std::string& left, const std::string& right)
{
	return left == right;
}


bool LLVMCatIntrinsics::stringNotEquals(const std::string& left, const std::string& right)
{
	return left != right;
}


std::string LLVMCatIntrinsics::stringAppend(const std::string& left, const std::string& right)
{
	return left + right;
}


std::string LLVMCatIntrinsics::floatToString(float number)
{
	return Tools::makeString(number);
}


std::string LLVMCatIntrinsics::intToString(int number)
{
	return Tools::makeString(number);
}


void LLVMCatIntrinsics::stringCopy(std::string* destination, const std::string& string)
{
	new (destination) std::string(string);
}


void LLVMCatIntrinsics::stringDestruct(std::string* target)
{
	target->~basic_string();
}


llvm::Value* LLVMCatIntrinsics::callStringDestruct(llvm::Value* target)
{
	return helper->callFunction(stringDestructFunctionType, reinterpret_cast<uintptr_t>(&stringDestruct), {target}, "callStringDestruct");
}


llvm::Value* LLVMCatIntrinsics::callStringEquals(llvm::Value* left, llvm::Value* right)
{
	return helper->callFunction(stringComparisonFunctionType, reinterpret_cast<uintptr_t>(&stringEquals), {left, right}, "callStringEquals");
}


llvm::Value* LLVMCatIntrinsics::callStringNotEquals(llvm::Value* left, llvm::Value* right)
{
	return helper->callFunction(stringComparisonFunctionType, reinterpret_cast<uintptr_t>(&stringNotEquals), {left, right}, "callStringNotEquals");
}


llvm::Value* LLVMCatIntrinsics::callStringAppend(llvm::Value* left, llvm::Value* right, LLVMCompileTimeContext* context)
{
	llvm::Value* stringObjectAllocation = helper->createStringAllocA(context, "stringAppendResult");
	llvm::CallInst* call = static_cast<llvm::CallInst*>(helper->callFunction(stringAppendFunctionType, reinterpret_cast<uintptr_t>(&stringAppend),  {stringObjectAllocation, left, right}, "stringAppend"));
	call->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
	call->addParamAttr(0, llvm::Attribute::AttrKind::NoAlias);
	call->addDereferenceableAttr(1, sizeof(std::string));
	call->addDereferenceableAttr(2, sizeof(std::string));
	call->addDereferenceableAttr(3, sizeof(std::string));
	
	return stringObjectAllocation;
}


llvm::Value* LLVMCatIntrinsics::callFloatToString(llvm::Value* number, LLVMCompileTimeContext* context)
{
	llvm::Value* stringObjectAllocation = helper->createStringAllocA(context, "floatToStringResult");
	llvm::CallInst* call = static_cast<llvm::CallInst*>(helper->callFunction(floatToStringFunctionType, reinterpret_cast<uintptr_t>(&floatToString),  {stringObjectAllocation, number}, "floatToString"));
	call->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
	call->addParamAttr(0, llvm::Attribute::AttrKind::NoAlias);
	call->addDereferenceableAttr(1, sizeof(std::string));
	return stringObjectAllocation;
}


llvm::Value* LLVMCatIntrinsics::callIntToString(llvm::Value* number, LLVMCompileTimeContext* context)
{
	llvm::Value* stringObjectAllocation = helper->createStringAllocA(context, "intToStringResult");
	llvm::CallInst* call = static_cast<llvm::CallInst*>(helper->callFunction(intToStringFunctionType, reinterpret_cast<uintptr_t>(&intToString),  {stringObjectAllocation, number}, "intToString"));
	call->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
	call->addParamAttr(0, llvm::Attribute::AttrKind::NoAlias);
	call->addDereferenceableAttr(1, sizeof(std::string));
	return stringObjectAllocation;
}


llvm::Value* LLVMCatIntrinsics::callStringCopy(llvm::Value* destinationMemory, llvm::Value* string)
{
	llvm::CallInst* call = static_cast<llvm::CallInst*>(helper->callFunction(stringCopyFunctionType, reinterpret_cast<uintptr_t>(&stringCopy),  {destinationMemory, string}, "stringCopy"));
	call->addDereferenceableAttr(1, sizeof(std::string));
	call->addDereferenceableAttr(2, sizeof(std::string));
	return helper->convertToPointer(destinationMemory, "stringCopy_Destination", LLVMTypes::stringPtrType);
}
