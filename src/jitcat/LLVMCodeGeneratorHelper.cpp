#include "LLVMCodeGeneratorHelper.h"
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


LLVMCodeGeneratorHelper::LLVMCodeGeneratorHelper(llvm::IRBuilder<>* builder):
	llvmContext(LLVMJit::get().getContext()),
	builder(builder),
	currentModule(nullptr)
{
}


Reflectable* LLVMCodeGeneratorHelper::getThisPointerFromContext(CatRuntimeContext* context)
{
	return context->getThisReference().getPointer()->getParentObject();
}


Reflectable* LLVMCodeGeneratorHelper::getCustomThisPointerFromContext(CatRuntimeContext* context)
{
	return context->getCustomThisReference().getPointer()->getParentObject();
}


llvm::Value* LLVMCodeGeneratorHelper::callGetThisPointer(llvm::Value* catRunTimeContext)
{
	return callFunction(LLVMTypes::functionRetPtrArgPtr, reinterpret_cast<uintptr_t>(&getThisPointerFromContext), {catRunTimeContext});
}


llvm::Value* LLVMCodeGeneratorHelper::callGetCustomThisPointer(llvm::Value* catRunTimeContext)
{
	return callFunction(LLVMTypes::functionRetPtrArgPtr, reinterpret_cast<uintptr_t>(&getCustomThisPointerFromContext), {catRunTimeContext});
}


llvm::Value* LLVMCodeGeneratorHelper::callFunction(llvm::FunctionType* functionType, uintptr_t functionAddress, const std::vector<llvm::Value*>& arguments)
{
	llvm::Value* functionAddressConstant = createIntPtrConstant(functionAddress);
	llvm::Value* addressAsPointer = builder->CreateIntToPtr(functionAddressConstant, llvm::PointerType::get(functionType, 0));
	return builder->CreateCall(functionType, addressAsPointer, arguments);
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, CatType parameterType, llvm::Value* argument)
{
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(parameterType));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument, toLLVMType(parameterType)));
	llvm::Function *fun = llvm::Intrinsic::getDeclaration(currentModule, intrinsic, paramaterTypes);
	return builder->CreateCall(fun, arguments);
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, CatType parameter1Type, CatType parameter2Type, llvm::Value* argument1, llvm::Value* argument2)
{
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(parameter1Type));
	paramaterTypes.push_back(toLLVMType(parameter2Type));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument1, toLLVMType(parameter1Type)));
	arguments.push_back(convertType(argument2, toLLVMType(parameter2Type)));
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
		case CatType::String:	return LLVMTypes::pointerType;
		case CatType::Object:	return LLVMTypes::pointerType;
		default:
		case CatType::Void:		return LLVMTypes::voidType;
	}
}


llvm::Value* LLVMCodeGeneratorHelper::convertType(llvm::Value* valueToConvert, llvm::Type* type)
{
	if (type == type->getInt1Ty(llvmContext))
	{
		//to 'boolean' type
		if (valueToConvert->getType() == type->getInt1Ty(llvmContext))
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == type->getInt32Ty(llvmContext))
		{
			llvm::Value* zero = llvm::ConstantInt::get(llvmContext, llvm::APInt(32, 0, true));
			return builder->CreateICmpSGT(valueToConvert, zero, "GreaterThanZero");
		}
		else if (valueToConvert->getType() == type->getFloatTy(llvmContext))
		{
			llvm::Value* zero = llvm::ConstantFP::get(llvmContext, llvm::APFloat(0.0));
			return builder->CreateFCmpUGT(valueToConvert, zero, "FGreaterThanZero");
		}

	}
	else if (type == type->getInt32Ty(llvmContext))
	{
		//to int type
		if (valueToConvert->getType() == type->getInt32Ty(llvmContext))
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == type->getInt1Ty(llvmContext))
		{
			return builder->CreateZExt(valueToConvert, LLVMTypes::intType, "ZeroExtended");
		}
		else if (valueToConvert->getType() == type->getFloatTy(llvmContext))
		{
			return builder->CreateFPToSI(valueToConvert, LLVMTypes::intType, "FloatToInt");
		}
	}
	else if (type == type->getFloatTy(llvmContext))
	{
		//to float type
		if (valueToConvert->getType() == type->getFloatTy(llvmContext))
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == type->getInt1Ty(llvmContext))
		{
			return builder->CreateUIToFP(valueToConvert, LLVMTypes::floatType, "BoolToFloat");
		}
		else if (valueToConvert->getType() == type->getInt32Ty(llvmContext))
		{
			return builder->CreateSIToFP(valueToConvert, LLVMTypes::floatType, "IntToFloat");
		}
	}
	LLVMJit::logError("ERROR: Invalid type conversion.");
	return nullptr;
}


llvm::Value* LLVMCodeGeneratorHelper::convertToPointer(llvm::Value* addressValue)
{
	return builder->CreateIntToPtr(addressValue, LLVMTypes::pointerType);
}


llvm::Value* LLVMCodeGeneratorHelper::convertToIntPtr(llvm::Value* llvmPointer)
{
	return builder->CreatePtrToInt(llvmPointer, LLVMTypes::uintPtrType);
}


bool LLVMCodeGeneratorHelper::isPointer(llvm::Value* value) const
{
	return value->getType() == LLVMTypes::pointerType;
}


bool LLVMCodeGeneratorHelper::isIntPtr(llvm::Value* value) const
{
	return value->getType() == LLVMTypes::uintPtrType;
}


llvm::Value* LLVMCodeGeneratorHelper::loadBasicType(llvm::Type* type, llvm::Value* addressValue)
{
	llvm::Value* addressAsPointer = builder->CreateIntToPtr(addressValue, llvm::PointerType::getUnqual(type));
	llvm::LoadInst* load = builder->CreateLoad(addressAsPointer);
	return load;
}


llvm::Value* LLVMCodeGeneratorHelper::loadPointerAtAddress(llvm::Value* addressValue)
{
	llvm::Value* addressAsPointer = builder->CreateIntToPtr(addressValue, llvm::PointerType::getUnqual(LLVMTypes::pointerType));
	llvm::LoadInst* load = builder->CreateLoad(addressAsPointer);
	return load;
}


llvm::Value* LLVMCodeGeneratorHelper::createAdd(llvm::Value* value1, llvm::Value* value2)
{
	return builder->CreateAdd(value1, value2);
}


llvm::Value* LLVMCodeGeneratorHelper::createIntPtrConstant(unsigned long long constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(sizeof(std::uintptr_t) * 8, constant, false));
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


llvm::LLVMContext& LLVMCodeGeneratorHelper::getContext()
{
	return llvmContext;
}


llvm::IRBuilder<>* LLVMCodeGeneratorHelper::getBuilder()
{
	return builder;
}
