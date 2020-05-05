/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/LLVMCodeGenerator.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMJit.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

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

LLVMCodeGeneratorHelper::LLVMCodeGeneratorHelper(llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* builder, llvm::Module* module):
	llvmContext(LLVMJit::get().getContext()),
	builder(builder),
	currentModule(module)
{
}


llvm::Value* LLVMCodeGeneratorHelper::createCall(llvm::FunctionType* functionType, uintptr_t functionAddress, const std::vector<llvm::Value*>& arguments, const std::string& functionName)
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
		llvm::Value* isNotNull = nullptr;
		if (valueToCheck->getType() != LLVMTypes::boolType)
		{
			isNotNull = builder->CreateIsNotNull(valueToCheck, "IsNotNull");
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


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& parameterType, llvm::Value* argument, LLVMCompileTimeContext* context)
{
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(parameterType));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument, toLLVMType(parameterType), context));
	llvm::Function *fun = llvm::Intrinsic::getDeclaration(currentModule, intrinsic, paramaterTypes);
	return builder->CreateCall(fun, arguments);
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& overload1Type, llvm::Value* argument1, llvm::Value* argument2, LLVMCompileTimeContext * context)
{
	std::vector<llvm::Type *> paramaterTypes;
	paramaterTypes.push_back(toLLVMType(overload1Type));
	std::vector<llvm::Value*> arguments;
	arguments.push_back(convertType(argument1, toLLVMType(overload1Type), context));
	arguments.push_back(convertType(argument2, toLLVMType(overload1Type), context));
	llvm::Function* fun = llvm::Intrinsic::getDeclaration(currentModule, intrinsic, paramaterTypes);
	return builder->CreateCall(fun, arguments);
}


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(llvm::Intrinsic::ID intrinsic, const CatGenericType& parameter1Type, const CatGenericType& parameter2Type, llvm::Value* argument1, llvm::Value* argument2, LLVMCompileTimeContext* context)
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


llvm::Type* LLVMCodeGeneratorHelper::toLLVMType(const CatGenericType& type)
{
	if		(type.isFloatType())						return LLVMTypes::floatType;
	else if (type.isIntType())							return LLVMTypes::intType;
	else if (type.isBoolType())							return LLVMTypes::boolType;
	else if (type.isStringType())						return LLVMTypes::stringPtrType;
	else if (type.isReflectableHandleType())			return LLVMTypes::pointerType;
	else if (type.isContainerType())					return LLVMTypes::pointerType;
	else if (type.isVoidType())							return LLVMTypes::voidType;
	else if (type.isEnumType())							return toLLVMType(type.getUnderlyingEnumType());
	else if (type.isReflectableObjectType())			
	{
		//This is a compound type. For now, just create a byte array type.
		return llvm::ArrayType::get(LLVMTypes::ucharType, type.getTypeSize());
	}
	else if (type.isPointerType())
	{
		if (type.getPointeeType()->isStringType())
		{
			return LLVMTypes::stringPtrType;
		}
		else if (type.getPointeeType()->isBasicType()
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
	builder->CreateStore(rValue, lValue);
}


llvm::Value* LLVMCodeGeneratorHelper::convertType(llvm::Value* valueToConvert, llvm::Type* type, LLVMCompileTimeContext* context)
{
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
		else if (valueToConvert->getType() == LLVMTypes::stringPtrType)
		{
			return createCall(context, &LLVMCatIntrinsics::stringToBoolean, {valueToConvert}, "stringToBoolean");
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
		else if (valueToConvert->getType() == LLVMTypes::stringPtrType)
		{
			return createCall(context, &LLVMCatIntrinsics::stringToInt, {valueToConvert}, "stringToInt");
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
		else if (valueToConvert->getType() == LLVMTypes::stringPtrType)
		{
			return createCall(context, &LLVMCatIntrinsics::stringToFloat, {valueToConvert}, "stringToFloat");
		}
	}
	else if (type == LLVMTypes::stringPtrType)
	{
		if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return builder->CreateSelect(valueToConvert, createOneStringPtrConstant(), createZeroStringPtrConstant(), "boolToString");
		}
		else if (valueToConvert->getType() == LLVMTypes::stringPtrType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			return createCall(context, &LLVMCatIntrinsics::floatToString, {valueToConvert}, "floatToString");
		}
		else if (valueToConvert->getType() == LLVMTypes::intType)
		{
			return createCall(context, &LLVMCatIntrinsics::intToString, {valueToConvert}, "intToString");
		}

	}
	LLVMJit::logError("ERROR: Invalid type conversion.");
	return nullptr;
}


llvm::Value* LLVMCodeGeneratorHelper::convertToPointer(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type)
{
	llvm::Value* intToPtr = builder->CreateIntToPtr(addressValue, type);
	intToPtr->setName(name);
	return intToPtr;
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::convertToPointer(llvm::Constant* addressConstant, const std::string& name, llvm::PointerType* type)
{
	return convertToPointer(static_cast<llvm::Value*>(addressConstant), name, type);
}


llvm::Value* LLVMCodeGeneratorHelper::convertToIntPtr(llvm::Value* llvmPointer, const std::string& name)
{
	llvm::Value* ptrToInt = builder->CreatePtrToInt(llvmPointer, LLVMTypes::uintPtrType);
	ptrToInt->setName(name);
	return ptrToInt;
}


bool jitcat::LLVM::LLVMCodeGeneratorHelper::isPointer(llvm::Type* type)
{
	return type == LLVMTypes::pointerType;
}


bool jitcat::LLVM::LLVMCodeGeneratorHelper::isStringPointer(llvm::Type* type)
{
	return type == LLVMTypes::stringPtrType;
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


bool LLVMCodeGeneratorHelper::isStringPointer(llvm::Value* value)
{
	return value->getType() == LLVMTypes::stringPtrType;
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
	return builder->CreateAdd(value1, value2, name);
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::createAdd(llvm::Value* value1, llvm::Constant* value2, const std::string& name)
{
	return builder->CreateAdd(value1, value2, name);
}


llvm::Constant* LLVMCodeGeneratorHelper::createZeroInitialisedConstant(llvm::Type* type)
{
	if		(type == LLVMTypes::boolType)		return createConstant(false);
	else if (type == LLVMTypes::floatType)		return createConstant(0.0f);
	else if (type == LLVMTypes::intType)		return createConstant(0);
	else if (type == LLVMTypes::charType)		return createCharConstant(0);
	//else if (type == LLVMTypes::stringPtrType)	return createNullPtrConstant(LLVMTypes::stringPtrType);
	//else if (type == LLVMTypes::pointerType)	return createNullPtrConstant(LLVMTypes::pointerType);
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


llvm::Value* LLVMCodeGeneratorHelper::createEmptyStringPtrConstant()
{
	return createPtrConstant(reinterpret_cast<uintptr_t>(&emptyString), "EmptyString", LLVMTypes::stringPtrType);
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::constantToValue(llvm::Constant* constant) const
{
	return constant;
}


void LLVMCodeGeneratorHelper::setCurrentModule(llvm::Module* module)
{
	currentModule = module;
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::createObjectAllocA(LLVMCompileTimeContext* context, const std::string& name, const CatGenericType& objectType, bool generateDestructorCall)
{
	llvm::Type* llvmObjectType = llvm::ArrayType::get(LLVMTypes::ucharType, objectType.getTypeSize());;
	llvm::BasicBlock* previousInsertBlock = builder->GetInsertBlock();
	bool currentBlockIsEntryBlock = &context->currentFunction->getEntryBlock() == previousInsertBlock;
	builder->SetInsertPoint(&context->currentFunction->getEntryBlock(), context->currentFunction->getEntryBlock().begin());
	llvm::AllocaInst* objectAllocation = builder->CreateAlloca(llvmObjectType, 0, nullptr);
	objectAllocation->setName(name);
	llvm::Value* objectAllocationAsIntPtr = builder->CreatePointerCast(objectAllocation, LLVMTypes::pointerType);
	Reflection::TypeInfo* typeInfo = objectType.getObjectType();
	assert(typeInfo != nullptr);

	llvm::Constant* typeInfoConstant = createIntPtrConstant(reinterpret_cast<uintptr_t>(typeInfo), Tools::append(name, "_typeInfo"));
	llvm::Value* typeInfoConstantAsIntPtr = convertToPointer(typeInfoConstant, Tools::append(name, "_typeInfoPtr"));
	llvm::BasicBlock* updatedBlock = builder->GetInsertBlock();

	if (generateDestructorCall)
	{
		std::string destructorName = Tools::append(name, "_destructor");
		assert(objectType.isDestructible());
		context->blockDestructorGenerators.push_back([=](){return createCall(context, &LLVMCatIntrinsics::placementDestructType, {objectAllocationAsIntPtr, typeInfoConstantAsIntPtr}, destructorName);});
	}
	if (currentBlockIsEntryBlock)
	{
		builder->SetInsertPoint(updatedBlock);
	}
	else
	{
		builder->SetInsertPoint(previousInsertBlock);
	}
	return objectAllocationAsIntPtr;

}


llvm::Value* LLVMCodeGeneratorHelper::createStringAllocA(LLVMCompileTimeContext* context, const std::string& name, bool generateDestructorCall)
{
	llvm::BasicBlock* previousInsertBlock = builder->GetInsertBlock();
	bool currentBlockIsEntryBlock = &context->currentFunction->getEntryBlock() == previousInsertBlock;
	builder->SetInsertPoint(&context->currentFunction->getEntryBlock(), context->currentFunction->getEntryBlock().begin());
	llvm::AllocaInst* stringObjectAllocation = builder->CreateAlloca(LLVMTypes::stringType, 0, nullptr);
	stringObjectAllocation->setName(name);

	llvm::BasicBlock* updatedBlock = builder->GetInsertBlock();
	if (generateDestructorCall)
	{
		context->blockDestructorGenerators.push_back([=](){return createCall(context, &LLVMCatIntrinsics::stringDestruct, {stringObjectAllocation}, "stringDestruct");});
	}
	if (currentBlockIsEntryBlock)
	{
		builder->SetInsertPoint(updatedBlock);
	}
	else
	{
		builder->SetInsertPoint(previousInsertBlock);
	}
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


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::generateFunctionCallReturnValueAllocation(const CatGenericType& returnType, const std::string& functionName, LLVMCompileTimeContext* context)
{
	//When calling a functions that returns an object by value, the object 
	//must be allocated on the stack and a pointer to that allocation should 
	//then be passed as the first argument to the function.
	//That argument should be marked SRet (structure return argument).
	//The function itself will return void.
	//Objects allocated on the stack will be destroyed before the function that contains this call returns.
	if (returnType.isStringType() || returnType.isReflectableObjectType())
	{
		std::string objectName = Tools::append(functionName, "_Result");
		if (returnType.isStringType())
		{
			return context->helper->createStringAllocA(context, objectName, true);
		}
		else
		{
			return context->helper->createObjectAllocA(context, objectName, returnType, true);
		}
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
		
		if (llvm::Type* argumentLLVMType = argumentValue->getType(); argumentLLVMType != expectedLLVMType)
		{
			//Check if the types can be converted.
			if (expectedLLVMType->isPointerTy())
			{
				//A pointer is expected
				if (argumentLLVMType == expectedLLVMType->getPointerElementType() && argumentLLVMType->isSingleValueType())
				{
					//The supplied argument is of the type pointed to by the expected type.
					//If it is a simple type, we can store it to memory and pass the address.
					llvm::AllocaInst* allocation = builder->CreateAlloca(argumentLLVMType);
					builder->CreateStore(argumentValue, allocation);
					argumentValue = allocation;
				}
			}
		}
		//If a parameter is passed by value, it should be copy constructed.
		if (functionParameterType.isReflectableObjectType())
		{
			llvm::Value* copyAllocation = context->helper->createObjectAllocA(context, Tools::append("Argument_", i, "_copy"), functionParameterType, false);
			const std::string& typeName = functionParameterType.getObjectType()->getTypeName();
			llvm::Constant* typeInfoConstant = createIntPtrConstant(reinterpret_cast<uintptr_t>(functionParameterType.getObjectType()), Tools::append(typeName, "_typeInfo"));
			llvm::Value* typeInfoConstantAsIntPtr = convertToPointer(typeInfoConstant, Tools::append(typeName, "_typeInfoPtr"));
			assert(functionParameterType.isCopyConstructible());
			createCall(context, &LLVMCatIntrinsics::placementCopyConstructType, {copyAllocation, argumentValue, typeInfoConstantAsIntPtr}, Tools::append(typeName, "_copyConstructor"));
			argumentValue = copyAllocation;
		}
		else if (functionParameterType.isStringType())
		{
			llvm::Value* copyAllocation = context->helper->createStringAllocA(context, Tools::append("Argument_", i, "_copy"), false);
			createCall(context, &LLVMCatIntrinsics::stringCopyConstruct, {copyAllocation, argumentValue}, "stringCopyConstruct");
			argumentValue = copyAllocation;
		}
		generatedArguments.push_back(argumentValue);
		generatedArgumentTypes.push_back(generatedArguments.back()->getType());
	}
}


llvm::FunctionType* LLVMCodeGeneratorHelper::createFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& argumentTypes)
{
	return llvm::FunctionType::get(returnType, argumentTypes, false);
}


llvm::Value* LLVMCodeGeneratorHelper::generateCall(LLVMCompileTimeContext* context, uintptr_t functionAddress, llvm::FunctionType* functionType, const std::vector<llvm::Value*>& arguments, bool isStructRet, const std::string& name)
{
	//QQQ
	std::vector<llvm::Value*> finalArguments(arguments);
	llvm::Value* structRetValue = nullptr;
	if (isStructRet)
	{
		structRetValue = createStringAllocA(context, name + "_Result", false);
		finalArguments.insert(finalArguments.begin(), structRetValue);
	}
	llvm::CallInst* call = static_cast<llvm::CallInst*>(createCall(functionType, functionAddress, finalArguments, name));

	unsigned int derefAttributeIndex = 1;
	auto iter = functionType->param_begin();
	if (isStructRet) ++iter;
	for (iter; iter != functionType->param_end(); ++iter)
	{
		if (*iter == LLVMTypes::stringPtrType)
		{
			call->addDereferenceableAttr(derefAttributeIndex, sizeof(std::string));
		}
		derefAttributeIndex++;
	}

	if (isStructRet)
	{
		call->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
		call->addParamAttr(0, llvm::Attribute::AttrKind::NoAlias);
		return structRetValue;
	}
	else
	{
		return call;
	}
}


llvm::Value* LLVMCodeGeneratorHelper::createZeroStringPtrConstant()
{
	return createPtrConstant(reinterpret_cast<uintptr_t>(&zeroString), "ZeroString", LLVMTypes::stringPtrType);
}


llvm::Value* LLVMCodeGeneratorHelper::createOneStringPtrConstant()
{
	return createPtrConstant(reinterpret_cast<uintptr_t>(&oneString), "OneString", LLVMTypes::stringPtrType);
}


const std::string LLVMCodeGeneratorHelper::emptyString = "";
const std::string LLVMCodeGeneratorHelper::zeroString = "0";
const std::string LLVMCodeGeneratorHelper::oneString = "1";