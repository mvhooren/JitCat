/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMJit.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Tools.h"

#include <llvm/IR/Constant.h>
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
			if (resultType == LLVMTypes::boolType)			return createConstant(false);
			else if (resultType == LLVMTypes::floatType)	return createConstant(0.0f);
			else if (resultType == LLVMTypes::intType)		return createConstant(0);
			else if (resultType == LLVMTypes::stringPtrType)return createPtrConstant(0, "nullString", LLVMTypes::stringPtrType);
			else if (resultType == LLVMTypes::pointerType)	return createPtrConstant(0, "nullptr");
			else if (resultType == LLVMTypes::voidType)		return (llvm::Value*)nullptr;
			else
			{
				assert(false);
				return (llvm::Value*)nullptr;
			}
		};
		return createOptionalNullCheckSelect(valueToCheck, codeGenIfNotNull, codeGenIfNull, context);
	}
	else
	{
		return codeGenIfNotNull(context);
	}
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
	if		(type.isFloatType())	return LLVMTypes::floatType;
	else if (type.isIntType())		return LLVMTypes::intType;
	else if (type.isBoolType())		return LLVMTypes::boolType;
	else if (type.isStringType())	return LLVMTypes::stringPtrType;
	else if (type.isObjectType())	return LLVMTypes::pointerType;
	else if (type.isContainerType())return LLVMTypes::pointerType;
	else							return LLVMTypes::voidType;
}

llvm::Type* LLVMCodeGeneratorHelper::toLLVMPtrType(const CatGenericType& type)
{
	return llvm::PointerType::get(toLLVMType(type), 0);
}


void LLVMCodeGeneratorHelper::writeToPointer(llvm::Value* lValue, llvm::Value* rValue)
{
	builder->CreateStore(rValue, lValue);
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


llvm::Value* LLVMCodeGeneratorHelper::createPtrConstant(unsigned long long address, const std::string& name, llvm::Type* pointerType)
{
	llvm::Value* constant = createIntPtrConstant(address, Tools::append(name, "_IntPtr"));
	return convertToPointer(constant, Tools::append(name, "_Ptr"), pointerType); 
}


llvm::Value* LLVMCodeGeneratorHelper::createEmptyStringPtrConstant()
{
	return createPtrConstant(reinterpret_cast<uintptr_t>(&emptyString), "EmptyString", LLVMTypes::stringPtrType);
}


void LLVMCodeGeneratorHelper::setCurrentModule(llvm::Module* module)
{
	currentModule = module;
}


llvm::Value* LLVMCodeGeneratorHelper::createStringAllocA(LLVMCompileTimeContext* context, const std::string& name)
{
	llvm::BasicBlock* previousInsertBlock = builder->GetInsertBlock();
	bool currentBlockIsEntryBlock = &context->currentFunction->getEntryBlock() == previousInsertBlock;
	builder->SetInsertPoint(&context->currentFunction->getEntryBlock(), context->currentFunction->getEntryBlock().begin());
	llvm::AllocaInst* stringObjectAllocation = builder->CreateAlloca(LLVMTypes::stringType, 0, nullptr);
	createCall(context, &LLVMCatIntrinsics::stringEmptyConstruct, {stringObjectAllocation}, "stringEmptyConstruct");
	stringObjectAllocation->setName(name);

	llvm::BasicBlock* updatedBlock = builder->GetInsertBlock();
	context->blockDestructorGenerators.push_back([=](){return createCall(context, &LLVMCatIntrinsics::stringDestruct, {stringObjectAllocation}, "stringDestruct");});
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


llvm::FunctionType* LLVMCodeGeneratorHelper::createFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& argumentTypes)
{
	return llvm::FunctionType::get(returnType, argumentTypes, false);
}


llvm::Value* LLVMCodeGeneratorHelper::generateCall(LLVMCompileTimeContext* context, uintptr_t functionAddress, llvm::FunctionType* functionType, const std::vector<llvm::Value*>& arguments, bool isStructRet, const std::string& name)
{
	std::vector<llvm::Value*> finalArguments(arguments);
	llvm::Value* structRetValue = nullptr;
	if (isStructRet)
	{
		structRetValue = createStringAllocA(context, name + "_Result");
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