#include "LLVMCodeGeneratorHelper.h"
#include "LLVMCompileTimeContext.h"
#include "LLVMJit.h"
#include "LLVMTypes.h"
#include "CatRuntimeContext.h"
#include "MemberReference.h"

//QQQ not portable, fix
#pragma warning(push, 0)        
//Disable warnings from llvm includes
#include <llvm\IR\Constant.h>
#include <llvm\IR\Function.h>
#include <llvm\IR\LegacyPassManager.h>
#include <llvm\IR\PassManager.h>
#include <llvm\IR\Intrinsics.h>
#include <llvm\IR\IRBuilder.h>
#include <llvm\IR\Module.h>
#include <llvm\IR\Value.h>
#include <llvm\IR\Verifier.h>
#include <llvm\Support\raw_ostream.h>
#include <llvm\Transforms\Scalar.h>
#include <llvm\Transforms\Scalar\GVN.h>
#pragma warning(pop)


LLVMCodeGeneratorHelper::LLVMCodeGeneratorHelper(llvm::IRBuilder<>* builder, llvm::Module* module):
	llvmContext(LLVMJit::get().getContext()),
	builder(builder),
	currentModule(module)
{
	intrinsics.reset(new LLVMCatIntrinsics(&llvmContext, this, module));
}


llvm::Value* LLVMCodeGeneratorHelper::callFunction(llvm::FunctionType* functionType, uintptr_t functionAddress, const std::vector<llvm::Value*>& arguments, const std::string& functionName)
{
	llvm::Value* functionAddressConstant = createIntPtrConstant(functionAddress, functionName + "_Address");
	functionAddressConstant->setName(functionName + "_IntPtr");
	llvm::Value* addressAsPointer = builder->CreateIntToPtr(functionAddressConstant, llvm::PointerType::get(functionType, 0));
	addressAsPointer->setName(functionName + "_Ptr");
	llvm::CallInst* callInstruction = builder->CreateCall(functionType, addressAsPointer, arguments);
	if (functionType->getReturnType() != LLVMTypes::voidType)
	{
		callInstruction->setName(functionName);
	}
	return callInstruction;
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, CatType parameterType, llvm::Value* argument, LLVMCompileTimeContext* context)
{
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(parameterType));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument, toLLVMType(parameterType), context));
	llvm::Function *fun = llvm::Intrinsic::getDeclaration(currentModule, intrinsic, paramaterTypes);
	return builder->CreateCall(fun, arguments);
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, CatType parameter1Type, CatType parameter2Type, llvm::Value* argument1, llvm::Value* argument2, LLVMCompileTimeContext* context)
{
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(parameter1Type));
	paramaterTypes.push_back(toLLVMType(parameter2Type));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument1, toLLVMType(parameter1Type), context));
	arguments.push_back(convertType(argument2, toLLVMType(parameter2Type), context));
	llvm::Function* fun = llvm::Intrinsic::getDeclaration(currentModule, intrinsic, paramaterTypes);
	return builder->CreateCall(fun, arguments);
}


llvm::Type* LLVMCodeGeneratorHelper::toLLVMType(CatType type)
{
	switch (type)
	{
		case CatType::Float:	return LLVMTypes::floatType;
		case CatType::Int:		return LLVMTypes::intType;
		case CatType::Bool:		return LLVMTypes::boolType;
		case CatType::String:	return LLVMTypes::stringType;
		case CatType::Object:	return LLVMTypes::pointerType;
		default:
		case CatType::Void:		return LLVMTypes::voidType;
	}
}


llvm::Value* LLVMCodeGeneratorHelper::convertType(llvm::Value* valueToConvert, llvm::Type* type, LLVMCompileTimeContext* context)
{
	if (type == LLVMTypes::boolType)
	{
		//to 'boolean' type
		if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::intType)
		{
			llvm::Value* zero = llvm::ConstantInt::get(llvmContext, llvm::APInt(32, 0, true));
			return builder->CreateICmpSGT(valueToConvert, zero, "GreaterThanZero");
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			llvm::Value* zero = llvm::ConstantFP::get(llvmContext, llvm::APFloat(0.0));
			return builder->CreateFCmpUGT(valueToConvert, zero, "FGreaterThanZero");
		}

	}
	else if (type == LLVMTypes::intType)
	{
		//to int type
		if (valueToConvert->getType() == LLVMTypes::intType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return builder->CreateZExt(valueToConvert, LLVMTypes::intType, "ZeroExtended");
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			return builder->CreateFPToSI(valueToConvert, LLVMTypes::intType, "FloatToInt");
		}
	}
	else if (type == LLVMTypes::floatType)
	{
		//to float type
		if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return builder->CreateUIToFP(valueToConvert, LLVMTypes::floatType, "BoolToFloat");
		}
		else if (valueToConvert->getType() == LLVMTypes::intType)
		{
			return builder->CreateSIToFP(valueToConvert, LLVMTypes::floatType, "IntToFloat");
		}
	}
	else if (type == LLVMTypes::stringPtrType)
	{
		if (valueToConvert->getType() == LLVMTypes::stringPtrType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			return intrinsics->callFloatToString(valueToConvert, context);
		}
		else if (valueToConvert->getType() == LLVMTypes::intType)
		{
			return intrinsics->callIntToString(valueToConvert, context);
		}

	}
	LLVMJit::logError("ERROR: Invalid type conversion.");
	return nullptr;
}


llvm::Value* LLVMCodeGeneratorHelper::convertToPointer(llvm::Value* addressValue, const std::string& name, llvm::Type* type)
{
	llvm::Value* intToPtr = builder->CreateIntToPtr(addressValue, type);
	intToPtr->setName(name);
	return intToPtr;
}


llvm::Value* LLVMCodeGeneratorHelper::convertToIntPtr(llvm::Value* llvmPointer, const std::string& name)
{
	llvm::Value* ptrToInt = builder->CreatePtrToInt(llvmPointer, LLVMTypes::uintPtrType);
	ptrToInt->setName(name);
	return ptrToInt;
}


bool LLVMCodeGeneratorHelper::isPointer(llvm::Value* value) const
{
	return value->getType() == LLVMTypes::pointerType;
}


bool LLVMCodeGeneratorHelper::isStringPointer(llvm::Value* value) const
{
	return value->getType() == LLVMTypes::stringPtrType;
}


bool LLVMCodeGeneratorHelper::isIntPtr(llvm::Value* value) const
{
	return value->getType() == LLVMTypes::uintPtrType;
}


llvm::Value* LLVMCodeGeneratorHelper::loadBasicType(llvm::Type* type, llvm::Value* addressValue, const std::string& name)
{
	llvm::Value* addressAsPointer = builder->CreateIntToPtr(addressValue, llvm::PointerType::getUnqual(type));
	addressAsPointer->setName(name + "_Ptr");
	llvm::LoadInst* load = builder->CreateLoad(addressAsPointer);
	load->setName(name);
	return load;
}


llvm::Value* LLVMCodeGeneratorHelper::loadPointerAtAddress(llvm::Value* addressValue, const std::string& name, llvm::Type* type)
{
	llvm::Value* addressAsPointer = builder->CreateIntToPtr(addressValue, llvm::PointerType::getUnqual(type));
	addressAsPointer->setName(name + "_Ptr");
	llvm::LoadInst* load = builder->CreateLoad(addressAsPointer);
	load->setName(name);
	return load;
}


llvm::Value* LLVMCodeGeneratorHelper::createAdd(llvm::Value* value1, llvm::Value* value2, const std::string& name)
{
	return builder->CreateAdd(value1, value2, name);
}


llvm::Value* LLVMCodeGeneratorHelper::createIntPtrConstant(unsigned long long constant, const std::string& name)
{
	llvm::ConstantInt* intConstant = llvm::ConstantInt::get(llvmContext, llvm::APInt(sizeof(std::uintptr_t) * 8, constant, false));
	intConstant->setName(name);
	return intConstant;
}


llvm::Value* LLVMCodeGeneratorHelper::createConstant(int constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(32, constant, true));
}


llvm::Value* LLVMCodeGeneratorHelper::createConstant(float constant)
{
	return llvm::ConstantFP::get(llvmContext, llvm::APFloat(constant));													
}


llvm::Value* LLVMCodeGeneratorHelper::createConstant(bool constant)
{
	return constant ? llvm::ConstantInt::getTrue(llvmContext) : llvm::ConstantInt::getFalse(llvmContext);
}


void LLVMCodeGeneratorHelper::setCurrentModule(llvm::Module* module)
{
	currentModule = module;
}


llvm::Value* LLVMCodeGeneratorHelper::createStringAllocA(LLVMCompileTimeContext* context, const std::string& name)
{
	llvm::AllocaInst* stringObjectAllocation = builder->CreateAlloca(LLVMTypes::stringType, 0, nullptr);
	stringObjectAllocation->setName(name);
	context->blockDestructorGenerators.push_back([=](){return intrinsics->callStringDestruct(stringObjectAllocation);});
	return stringObjectAllocation;
}


void LLVMCodeGeneratorHelper::generateBlockDestructors(LLVMCompileTimeContext* context)
{
	for (auto& iter : context->blockDestructorGenerators)
	{
		iter();
	}
}


llvm::LLVMContext& LLVMCodeGeneratorHelper::getContext()
{
	return llvmContext;
}


llvm::IRBuilder<>* LLVMCodeGeneratorHelper::getBuilder()
{
	return builder;
}
