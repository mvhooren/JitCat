/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/Configuration.h"
#include "jitcat/LLVMCodeGenerator.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMJit.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

#include <cassert>

#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

using namespace jitcat;
using namespace jitcat::LLVM;

LLVMCodeGeneratorHelper::LLVMCodeGeneratorHelper(LLVMCodeGenerator* codeGenerator):
	codeGenerator(codeGenerator),
	llvmContext(LLVMJit::get().getContext())
{
}


llvm::Value* LLVMCodeGeneratorHelper::createCall(llvm::FunctionType* functionType, uintptr_t functionAddress, const std::vector<llvm::Value*>& arguments, const std::string& functionName)
{
	llvm::CallInst* callInstruction = nullptr;
	if (functionAddress != 0)
	{
		//Call the function through the provided address
		llvm::Value* functionAddressConstant = createIntPtrConstant(functionAddress, functionName + "_Address");
		functionAddressConstant->setName(functionName + "_IntPtr");
		llvm::Value* addressAsPointer = codeGenerator->getBuilder()->CreateIntToPtr(functionAddressConstant, llvm::PointerType::get(functionType, 0));
		addressAsPointer->setName(functionName + "_Ptr");
		callInstruction = codeGenerator->getBuilder()->CreateCall(functionType, addressAsPointer, arguments);
	}
	else
	{
		//Call the function by symbol.

		//First try and find the function in the current module.
		llvm::Function* function = codeGenerator->getCurrentModule()->getFunction(functionName);
		if (function == nullptr)
		{
			//If the function was not found, create the function signature and add it to the module. 
			function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, functionName.c_str(), codeGenerator->getCurrentModule());
		}
		else
		{
			//Check if the function has the correct signature
			assert(function->getFunctionType() == functionType);
		}
		callInstruction = getBuilder()->CreateCall(function, arguments);
	}
	if (functionType->getReturnType() != LLVMTypes::voidType)
	{
		callInstruction->setName(functionName);
	}
	return callInstruction;
}


llvm::Value* LLVMCodeGeneratorHelper::createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, llvm::Type* resultType, LLVMCompileTimeContext* context)
{
	if (context->options.enableDereferenceNullChecks)
	{
		auto codeGenIfNull = [=](LLVMCompileTimeContext* context)
		{
			return createZeroInitialisedConstant(resultType);
		};
		return createOptionalNullCheckSelect(valueToCheck, codeGenIfNotNull, codeGenIfNull, context);
	}
	else
	{
		return codeGenIfNotNull(context);
	}
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value * (LLVMCompileTimeContext*)> codeGenIfNotNull, llvm::PointerType* resultType, LLVMCompileTimeContext* context)
{
	return createOptionalNullCheckSelect(valueToCheck, codeGenIfNotNull, static_cast<llvm::Type*>(resultType), context);
}


llvm::Value* LLVMCodeGeneratorHelper::createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNull, LLVMCompileTimeContext* context)
{
	if (context->options.enableDereferenceNullChecks)
	{
		auto builder = codeGenerator->getBuilder();
		llvm::Value* isNotNull = nullptr;
		if (valueToCheck->getType() != LLVMTypes::boolType)
		{
			std::string name = "IsNotNull";
			if (valueToCheck->getName() != "")
			{
				name = valueToCheck->getName().str() + name;
			}

			isNotNull = builder->CreateIsNotNull(valueToCheck, name);
		}
		else
		{
			isNotNull = valueToCheck;
		}
		llvm::Function* currentFunction = builder->GetInsertBlock()->getParent();

		llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(llvmContext, "thenIsNotNull", currentFunction);
		llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(llvmContext, "elseIsNull");
		llvm::BasicBlock* continuationBlock = llvm::BasicBlock::Create(llvmContext, "ifcont");
		llvm::BranchInst* branchInst = builder->CreateCondBr(isNotNull, thenBlock, elseBlock);

		llvm::MDBuilder metadataBuilder(llvmContext);
		llvm::MDNode* branchPredictNode = metadataBuilder.createBranchWeights(2000, 1);
		branchInst->setMetadata(llvm::LLVMContext::MD_prof, branchPredictNode);

		builder->SetInsertPoint(thenBlock);
		llvm::Value* thenResult = codeGenIfNotNull(context);
		builder->CreateBr(continuationBlock);
		thenBlock = builder->GetInsertBlock();

		currentFunction->getBasicBlockList().push_back(elseBlock);
		builder->SetInsertPoint(elseBlock);
		if (thenResult->getType() != LLVMTypes::voidType)
		{
			llvm::Value* elseResult = codeGenIfNull(context);
			builder->CreateBr(continuationBlock);
			// codegen of 'Else' can change the current block, update ElseBB for the PHI.
			elseBlock = builder->GetInsertBlock();
			currentFunction->getBasicBlockList().push_back(continuationBlock);
			builder->SetInsertPoint(continuationBlock);
			llvm::PHINode* phiNode = builder->CreatePHI(thenResult->getType(), 2, "ifResult");

			phiNode->addIncoming(thenResult, thenBlock);
			phiNode->addIncoming(elseResult, elseBlock);
			return phiNode;
		}
		else
		{
			codeGenIfNull(context);
			builder->CreateBr(continuationBlock);
			// codegen of 'Else' can change the current block, update ElseBB for the PHI.
			elseBlock = builder->GetInsertBlock();
			currentFunction->getBasicBlockList().push_back(continuationBlock);
			builder->SetInsertPoint(continuationBlock);
			return nullptr;
		}
	}
	else
	{
		return codeGenIfNotNull(context);
	}
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& parameterType, 
													llvm::Value* argument, const CatGenericType& argumentType, 
													LLVMCompileTimeContext* context)
{
	auto builder = codeGenerator->getBuilder();
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(parameterType));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument, argumentType, parameterType, context));
	llvm::Function *fun = llvm::Intrinsic::getDeclaration(codeGenerator->getCurrentModule(), intrinsic, paramaterTypes);
	return builder->CreateCall(fun, arguments);
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& overload1Type, 
													llvm::Value* argument1, const CatGenericType& argument1Type, 
													llvm::Value* argument2, const CatGenericType& argument2Type, 
													LLVMCompileTimeContext * context)
{
	auto builder = codeGenerator->getBuilder();
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(overload1Type));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument1, argument1Type, overload1Type, context));
	arguments.push_back(convertType(argument2, argument2Type, overload1Type, context));
	llvm::Function* fun = llvm::Intrinsic::getDeclaration(codeGenerator->getCurrentModule(), intrinsic, paramaterTypes);
	return builder->CreateCall(fun, arguments);
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& parameter1Type, const CatGenericType& parameter2Type, 
													llvm::Value* argument1, const CatGenericType& argument1Type, 
													llvm::Value* argument2, const CatGenericType& argument2Type,
													LLVMCompileTimeContext* context)
{
	auto builder = codeGenerator->getBuilder();
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(parameter1Type));
	paramaterTypes.push_back(toLLVMType(parameter2Type));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument1, argument1Type, parameter1Type, context));
	arguments.push_back(convertType(argument2, argument2Type, parameter2Type, context));
	llvm::Function* fun = llvm::Intrinsic::getDeclaration(codeGenerator->getCurrentModule(), intrinsic, paramaterTypes);
	return builder->CreateCall(fun, arguments);
}


llvm::Type* LLVMCodeGeneratorHelper::toLLVMType(const CatGenericType& type)
{
	if		(type.isFloatType())						return LLVMTypes::floatType;
	else if	(type.isDoubleType())						return LLVMTypes::doubleType;
	else if (type.isIntType())							return LLVMTypes::intType;
	else if (type.isBoolType())							return LLVMTypes::boolType;
	else if (type.isReflectableHandleType())			return LLVMTypes::pointerType;
	else if (type.isVoidType())							return LLVMTypes::voidType;
	else if (type.isEnumType())							return toLLVMType(type.getUnderlyingEnumType());
	else if (type.isReflectableObjectType())			
	{
		//This is a compound type. For now, just create a byte array type.
		return llvm::ArrayType::get(LLVMTypes::ucharType, type.getTypeSize());
	}
	else if (type.isPointerType())
	{
		if (type.getPointeeType()->isBasicType()
			|| type.getPointeeType()->isPointerType())
		{
			llvm::Type* pointee = toLLVMType(*type.getPointeeType());
			return pointee->getPointerTo();
		}
		else
		{
			return LLVMTypes::pointerType;
		}

	}
	else
	{
		//Unknown type. Add it to this function.
		assert(false);
		return LLVMTypes::voidType;
	}
}

llvm::PointerType* LLVMCodeGeneratorHelper::toLLVMPtrType(const CatGenericType& type)
{
	return llvm::PointerType::get(toLLVMType(type), 0);
}


void LLVMCodeGeneratorHelper::writeToPointer(llvm::Value* lValue, llvm::Value* rValue)
{
	codeGenerator->getBuilder()->CreateStore(rValue, lValue);
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::convertType(llvm::Value* valueToConvert, const CatGenericType& fromType, const CatGenericType& toType, LLVMCompileTimeContext* context)
{
	assert(toLLVMType(fromType) == valueToConvert->getType());
	if (fromType.compare(toType, false, true))
	{
		return valueToConvert;
	}
	else if (fromType.isBasicType() && toType.isBasicType())
	{
		return convertType(valueToConvert, toLLVMType(toType), context);
	}
	else if (toType.isStringType())
	{
		return convertToString(valueToConvert, context);
	}
	else if (fromType.isStringType())
	{
		if (toType.isBoolType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToBoolean, {valueToConvert}, "stringToBoolean");
		}
		else if (toType.isIntType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToInt, {valueToConvert}, "stringToInt");
		}
		else if (toType.isFloatType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToFloat, {valueToConvert}, "stringToFloat");
		}
		else if (toType.isDoubleType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToDouble, {valueToConvert}, "stringToDouble");
		}
	}
	assert(false);
	return valueToConvert;
}


llvm::Value* LLVMCodeGeneratorHelper::convertType(llvm::Value* valueToConvert, llvm::Type* type, LLVMCompileTimeContext* context)
{
	auto builder = codeGenerator->getBuilder();
	if (valueToConvert->getType() == type)
	{
		return valueToConvert;
	}
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
			llvm::Value* zero = llvm::ConstantFP::get(llvmContext, llvm::APFloat(0.0f));
			return builder->CreateFCmpUGT(valueToConvert, zero, "FGreaterThanZero");
		}
		else if (valueToConvert->getType() == LLVMTypes::doubleType)
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
		else if (valueToConvert->getType() == LLVMTypes::doubleType)
		{
			return builder->CreateFPToSI(valueToConvert, LLVMTypes::intType, "DoubleToInt");
		}
	}
	else if (type == LLVMTypes::floatType)
	{
		//to float type
		if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::doubleType)
		{
			return builder->CreateFPCast(valueToConvert, LLVMTypes::floatType, "DoubleToFloat");
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
	else if (type == LLVMTypes::doubleType)
	{
		//to float type
		if (valueToConvert->getType() == LLVMTypes::doubleType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			return builder->CreateFPCast(valueToConvert, LLVMTypes::doubleType, "FloatToDouble");
		}
		else if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return builder->CreateUIToFP(valueToConvert, LLVMTypes::doubleType, "BoolToDouble");
		}
		else if (valueToConvert->getType() == LLVMTypes::intType)
		{
			return builder->CreateSIToFP(valueToConvert, LLVMTypes::doubleType, "IntToDouble");
		}
	}
	LLVMJit::logError("ERROR: Invalid type conversion.");
	assert(false);
	return nullptr;
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::convertToString(llvm::Value* valueToConvert, LLVMCompileTimeContext* context)
{
	if (valueToConvert->getType() == LLVMTypes::boolType)
	{
		return createIntrinsicCall(context, &LLVMCatIntrinsics::boolToString, {valueToConvert}, "boolToString");
	}
	else if (valueToConvert->getType() == LLVMTypes::doubleType)
	{
		return createIntrinsicCall(context, &LLVMCatIntrinsics::doubleToString, {valueToConvert}, "doubleToString");
	}
	else if (valueToConvert->getType() == LLVMTypes::floatType)
	{
		return createIntrinsicCall(context, &LLVMCatIntrinsics::floatToString, {valueToConvert}, "floatToString");
	}
	else if (valueToConvert->getType() == LLVMTypes::intType)
	{
		return createIntrinsicCall(context, &LLVMCatIntrinsics::intToString, {valueToConvert}, "intToString");
	}
	else
	{
		return valueToConvert;
	}
}


llvm::Value* LLVMCodeGeneratorHelper::convertToPointer(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type)
{
	llvm::Value* intToPtr =  codeGenerator->getBuilder()->CreateIntToPtr(addressValue, type);
	intToPtr->setName(name);
	return intToPtr;
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::convertToPointer(llvm::Constant* addressConstant, const std::string& name, llvm::PointerType* type)
{
	return convertToPointer(static_cast<llvm::Value*>(addressConstant), name, type);
}


llvm::Value* LLVMCodeGeneratorHelper::convertToIntPtr(llvm::Value* llvmPointer, const std::string& name)
{
	llvm::Value* ptrToInt = codeGenerator->getBuilder()->CreatePtrToInt(llvmPointer, LLVMTypes::uintPtrType);
	ptrToInt->setName(name);
	return ptrToInt;
}


bool jitcat::LLVM::LLVMCodeGeneratorHelper::isPointer(llvm::Type* type)
{
	return type == LLVMTypes::pointerType;
}


bool jitcat::LLVM::LLVMCodeGeneratorHelper::isIntPtr(llvm::Type* type)
{
	return type == LLVMTypes::uintPtrType;
}


bool jitcat::LLVM::LLVMCodeGeneratorHelper::isInt(llvm::Type* type)
{
	return type == LLVMTypes::intType;
}


bool LLVMCodeGeneratorHelper::isPointer(llvm::Value* value)
{
	return value->getType() == LLVMTypes::pointerType;
}


bool LLVMCodeGeneratorHelper::isIntPtr(llvm::Value* value)
{
	return value->getType() == LLVMTypes::uintPtrType;
}


bool jitcat::LLVM::LLVMCodeGeneratorHelper::isInt(llvm::Value* value)
{
	return value->getType() == LLVMTypes::intType;
}


llvm::Value* LLVMCodeGeneratorHelper::loadBasicType(llvm::Type* type, llvm::Value* addressValue, const std::string& name)
{
	auto builder = codeGenerator->getBuilder();
	llvm::Value* addressAsPointer = builder->CreateIntToPtr(addressValue, llvm::PointerType::getUnqual(type));
	addressAsPointer->setName(name + "_Ptr");
	llvm::LoadInst* load = builder->CreateLoad(addressAsPointer);
	load->setName(name);
	return load;
}

llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::loadBasicType(llvm::Type* type, llvm::Constant* addressValue, const std::string& name)
{
	return loadBasicType(type, static_cast<llvm::Value*>(addressValue), name);
}


llvm::Value* LLVMCodeGeneratorHelper::loadPointerAtAddress(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type)
{
	auto builder = codeGenerator->getBuilder();
	llvm::Value* addressAsPointer = builder->CreateIntToPtr(addressValue, llvm::PointerType::getUnqual(type));
	addressAsPointer->setName(name + "_Ptr");
	llvm::LoadInst* load = builder->CreateLoad(addressAsPointer);
	load->setName(name);
	return load;
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::loadPointerAtAddress(llvm::Constant* addressValue, const std::string& name, llvm::PointerType* type)
{
	return loadPointerAtAddress(static_cast<llvm::Value*>(addressValue), name, type);
}


llvm::Value* LLVMCodeGeneratorHelper::createAdd(llvm::Value* value1, llvm::Value* value2, const std::string& name)
{
	return codeGenerator->getBuilder()->CreateAdd(value1, value2, name);
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::createAdd(llvm::Value* value1, llvm::Constant* value2, const std::string& name)
{
	return codeGenerator->getBuilder()->CreateAdd(value1, value2, name);
}


llvm::Constant* LLVMCodeGeneratorHelper::createZeroInitialisedConstant(llvm::Type* type)
{
	if		(type == LLVMTypes::boolType)		return createConstant(false);
	else if (type == LLVMTypes::floatType)		return createConstant(0.0f);
	else if (type == LLVMTypes::doubleType)		return createConstant(0.0);
	else if (type == LLVMTypes::intType)		return createConstant(0);
	else if (type == LLVMTypes::charType)		return createCharConstant(0);
	else if (type == LLVMTypes::voidType)		return (llvm::Constant*)nullptr;
	else if (type->isArrayTy())					return createZeroInitialisedArrayConstant(static_cast<llvm::ArrayType*>(type));
	else if (type->isPointerTy())
	{
		return createNullPtrConstant(static_cast<llvm::PointerType*>(type));
	}
	else
	{
		assert(false);
		return (llvm::Constant*)nullptr;
	}
}


llvm::Constant* LLVMCodeGeneratorHelper::createIntPtrConstant(unsigned long long constant, const std::string& name)
{
	llvm::ConstantInt* intConstant = llvm::ConstantInt::get(llvmContext, llvm::APInt(sizeof(std::uintptr_t) * 8, constant, false));
	intConstant->setName(name);
	return intConstant;
}


llvm::Constant* jitcat::LLVM::LLVMCodeGeneratorHelper::createCharConstant(char constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(8, constant, true));
}


llvm::Constant* jitcat::LLVM::LLVMCodeGeneratorHelper::createUCharConstant(unsigned char constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(8, constant, false));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(int constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(32, constant, true));
}


llvm::Constant* jitcat::LLVM::LLVMCodeGeneratorHelper::createConstant(double constant)
{
	return llvm::ConstantFP::get(llvmContext, llvm::APFloat(constant));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(float constant)
{
	return llvm::ConstantFP::get(llvmContext, llvm::APFloat(constant));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(bool constant)
{
	return constant ? llvm::ConstantInt::getTrue(llvmContext) : llvm::ConstantInt::getFalse(llvmContext);
}


llvm::Constant* jitcat::LLVM::LLVMCodeGeneratorHelper::createNullPtrConstant(llvm::PointerType* pointerType)
{
	return llvm::ConstantPointerNull::get(pointerType);
}


llvm::Value* LLVMCodeGeneratorHelper::createPtrConstant(unsigned long long address, const std::string& name, llvm::PointerType* pointerType)
{
	llvm::Constant* constant = createIntPtrConstant(address, Tools::append(name, "_IntPtr"));
	return convertToPointer(constant, Tools::append(name, "_Ptr"), pointerType); 
}


llvm::Constant* jitcat::LLVM::LLVMCodeGeneratorHelper::createZeroInitialisedArrayConstant(llvm::ArrayType* arrayType)
{
	uint64_t numElements = arrayType->getArrayNumElements();
	std::vector<llvm::Constant*> constants;
	for (std::size_t i = 0; i < numElements; ++i)
	{
		constants.push_back(createZeroInitialisedConstant(arrayType->getArrayElementType()));
	}
	return llvm::ConstantArray::get(arrayType, constants);
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::constantToValue(llvm::Constant* constant) const
{
	return constant;
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::createObjectAllocA(LLVMCompileTimeContext* context, const std::string& name, const CatGenericType& objectType, bool generateDestructorCall)
{
	auto builder = codeGenerator->getBuilder();
	llvm::Type* llvmObjectType = llvm::ArrayType::get(LLVMTypes::ucharType, objectType.getTypeSize());
	llvm::BasicBlock* previousInsertBlock = builder->GetInsertBlock();
	bool currentBlockIsEntryBlock = &context->currentFunction->getEntryBlock() == previousInsertBlock;
	builder->SetInsertPoint(&context->currentFunction->getEntryBlock(), context->currentFunction->getEntryBlock().begin());
	llvm::AllocaInst* objectAllocation = builder->CreateAlloca(llvmObjectType, 0, nullptr);
	objectAllocation->setName(name);
	
	llvm::Value* objectAllocationAsIntPtr = builder->CreatePointerCast(objectAllocation, LLVMTypes::pointerType);

	llvm::BasicBlock* updatedBlock = builder->GetInsertBlock();

	if (currentBlockIsEntryBlock)
	{
		builder->SetInsertPoint(updatedBlock);
	}
	else
	{
		builder->SetInsertPoint(previousInsertBlock);
	}
	if (generateDestructorCall)
	{
		Reflection::TypeInfo* typeInfo = objectType.getObjectType();
		assert(typeInfo != nullptr);
		llvm::Constant* typeInfoConstant = createIntPtrConstant(reinterpret_cast<uintptr_t>(typeInfo), Tools::append(name, "_typeInfo"));
		llvm::Value* typeInfoConstantAsIntPtr = convertToPointer(typeInfoConstant, Tools::append(name, "_typeInfoPtr"));
		assert(objectType.isDestructible());
		context->blockDestructorGenerators.push_back([=]()
			{
				return createIntrinsicCall(context, &LLVMCatIntrinsics::placementDestructType, {objectAllocationAsIntPtr, typeInfoConstantAsIntPtr}, "placementDestructType");
			});
	}
	return objectAllocationAsIntPtr;

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
	return codeGenerator->getBuilder();
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::generateFunctionCallReturnValueAllocation(const CatGenericType& returnType, const std::string& functionName, LLVMCompileTimeContext* context)
{
	//When calling a functions that returns an object by value, the object 
	//must be allocated on the stack and a pointer to that allocation should 
	//then be passed as the first argument to the function.
	//That argument should be marked SRet (structure return argument).
	//The function itself will return void.
	//Objects allocated on the stack will be destroyed before the function that contains this call returns.
	if (returnType.isReflectableObjectType())
	{
		std::string objectName = Tools::append(functionName, "_Result");
		return context->helper->createObjectAllocA(context, objectName, returnType, true);
	}
	return nullptr;
}


void LLVMCodeGeneratorHelper::generateFunctionCallArgumentEvalatuation(const std::vector<const jitcat::AST::CatTypedExpression*>& arguments,
																	   const std::vector<CatGenericType>& expectedArgumentTypes, 
																	   std::vector<llvm::Value*>& generatedArguments,
																	   std::vector<llvm::Type*>& generatedArgumentTypes,
																	   LLVMCodeGenerator* generator, LLVMCompileTimeContext* context)
{
	assert(arguments.size() == expectedArgumentTypes.size());
	for (std::size_t i = 0; i < arguments.size(); i++)
	{
		llvm::Value* argumentValue = generator->generate(arguments[i], context);
		const CatGenericType& functionParameterType = expectedArgumentTypes[i];
		llvm::Type* expectedLLVMType = toLLVMType(functionParameterType);
		
		argumentValue = convertIndirection(argumentValue, expectedLLVMType);

		//If a parameter is passed by value, it should be copy constructed.
		argumentValue = copyConstructIfValueType(argumentValue, functionParameterType, context, Tools::makeString(i));

		generatedArguments.push_back(argumentValue);
		generatedArgumentTypes.push_back(generatedArguments.back()->getType());
	}
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::generateStaticFunctionCall(const jitcat::CatGenericType& returnType, const std::vector<llvm::Value*>& argumentList, const std::vector<llvm::Type*>& argumentTypes, LLVMCompileTimeContext* context, uintptr_t functionAddress, const std::string& functionName, llvm::Value* returnedObjectAllocation)
{
	if (returnType.isReflectableObjectType())
	{
		llvm::FunctionType* functionType = llvm::FunctionType::get(LLVMTypes::voidType, argumentTypes, false);
		llvm::CallInst* call = static_cast<llvm::CallInst*>(createCall(functionType, functionAddress, argumentList, functionName));
		call->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
		call->addDereferenceableAttr(1 , returnType.getTypeSize());
		return returnedObjectAllocation;
	}
	else
	{
		llvm::Type* returnLLVMType = toLLVMType(returnType);
		llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
		return createCall(functionType, functionAddress, argumentList, functionName);
	}
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::convertIndirection(llvm::Value* value, llvm::Type* expectedType)
{
	auto builder = codeGenerator->getBuilder();
	if (llvm::Type* argumentLLVMType = value->getType(); argumentLLVMType != expectedType)
	{
		//Check if the types can be converted.
		if (expectedType->isPointerTy())
		{
			//A pointer is expected
			if (argumentLLVMType == expectedType->getPointerElementType() && argumentLLVMType->isSingleValueType())
			{
				//The supplied argument is of the type pointed to by the expected type.
				//If it is a simple type, we can store it to memory and pass the address.
				llvm::AllocaInst* allocation = builder->CreateAlloca(argumentLLVMType);
				builder->CreateStore(value, allocation);
				return allocation;
			}
		}
	}
	return value;
}

llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::copyConstructIfValueType(llvm::Value* value, const CatGenericType& type, LLVMCompileTimeContext* context, const std::string& valueName)
{
	//If a parameter is passed by value, it should be copy constructed.
	if (type.isReflectableObjectType())
	{
		llvm::Value* copyAllocation = context->helper->createObjectAllocA(context, Tools::append("Argument_", valueName, "_copy"), type, Configuration::callerDestroysTemporaryArguments);
		const std::string& typeName = type.getObjectType()->getTypeName();
		llvm::Constant* typeInfoConstant = createIntPtrConstant(reinterpret_cast<uintptr_t>(type.getObjectType()), Tools::append(typeName, "_typeInfo"));
		llvm::Value* typeInfoConstantAsIntPtr = convertToPointer(typeInfoConstant, Tools::append(typeName, "_typeInfoPtr"));
		assert(type.isCopyConstructible());
		createIntrinsicCall(context, &LLVMCatIntrinsics::placementCopyConstructType, {copyAllocation, value, typeInfoConstantAsIntPtr}, Tools::append(typeName, "_copyConstructor"));
		return copyAllocation;
	}
	return value;
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::generateIntrinsicCall(jitcat::Reflection::StaticFunctionInfo* functionInfo, std::vector<llvm::Value*>& arguments, LLVMCompileTimeContext* context)
{
	//Define the function in the runtime library.
	llvm::JITSymbolFlags functionFlags;
	functionFlags |= llvm::JITSymbolFlags::Callable;
	functionFlags |= llvm::JITSymbolFlags::Exported;
	functionFlags |= llvm::JITSymbolFlags::Absolute;
	functionFlags |= llvm::JITSymbolFlags::Weak;
	llvm::orc::SymbolMap intrinsicSymbols;
	llvm::JITTargetAddress address = functionInfo->getFunctionAddress();
	intrinsicSymbols[codeGenerator->executionSession->intern(functionInfo->getNormalFunctionName())] = llvm::JITEvaluatedSymbol(address, functionFlags);
	llvm::cantFail(codeGenerator->runtimeLibraryDyLib->define(llvm::orc::absoluteSymbols(intrinsicSymbols)));
	
	const std::vector<CatGenericType>& argumentTypes = functionInfo->getArgumentTypes();
	assert(arguments.size() == argumentTypes.size());
	llvm::Value* returnValueAllocation = generateFunctionCallReturnValueAllocation(functionInfo->getReturnType(), functionInfo->getNormalFunctionName(), context);
	std::vector<llvm::Type*> argumentLLVMTypes;
	for (std::size_t i = 0; i < arguments.size(); ++i)
	{
		arguments[i] = convertIndirection(arguments[i], toLLVMType(argumentTypes[i]));
		arguments[i] = copyConstructIfValueType(arguments[i], argumentTypes[i], context, Tools::makeString(i)); 
		argumentLLVMTypes.push_back(arguments[i]->getType());
	}
	if (returnValueAllocation != nullptr)
	{
		arguments.insert(arguments.begin(), returnValueAllocation);
		argumentLLVMTypes.insert(argumentLLVMTypes.begin(), returnValueAllocation->getType());
	}

	return generateStaticFunctionCall(functionInfo->getReturnType(), arguments, argumentLLVMTypes, context, 0, functionInfo->getNormalFunctionName(), returnValueAllocation);
}


llvm::Value* LLVMCodeGeneratorHelper::createZeroStringPtrConstant()
{
	return createPtrConstant(reinterpret_cast<uintptr_t>(&zeroString), "ZeroString", LLVMTypes::pointerType);
}


llvm::Value* LLVMCodeGeneratorHelper::createOneStringPtrConstant()
{
	return createPtrConstant(reinterpret_cast<uintptr_t>(&oneString), "OneString", LLVMTypes::pointerType);
}


const Configuration::CatString LLVMCodeGeneratorHelper::emptyString = Configuration::CatString();
const Configuration::CatString LLVMCodeGeneratorHelper::zeroString = "0";
const Configuration::CatString LLVMCodeGeneratorHelper::oneString = "1";