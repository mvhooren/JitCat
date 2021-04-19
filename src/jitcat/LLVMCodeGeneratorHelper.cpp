/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatRange.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
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
using namespace jitcat::Reflection;

LLVMCodeGeneratorHelper::LLVMCodeGeneratorHelper(LLVMCodeGenerator* codeGenerator):
	codeGenerator(codeGenerator),
	llvmContext(LLVMJit::get().getContext())
{
}


llvm::Value* LLVMCodeGeneratorHelper::createCall(llvm::FunctionType* functionType, const std::vector<llvm::Value*>& arguments, bool isThisCall, 
												 const std::string& mangledFunctionName, const std::string& shortFunctionName)
{
	llvm::CallInst* callInstruction = nullptr;
	//First try and find the function in the current module.
	llvm::Function* function = codeGenerator->getCurrentModule()->getFunction(mangledFunctionName);
	if (function == nullptr)
	{
		//If the function was not found, create the function signature and add it to the module. 
		function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, mangledFunctionName.c_str(), codeGenerator->getCurrentModule());
		if (Configuration::useThisCall && isThisCall)
		{
			function->setCallingConv(llvm::CallingConv::X86_ThisCall);
		}
	}
	else
	{
		//Check if the function has the correct signature
		assert(function->getFunctionType() == functionType);
	}
	callInstruction = getBuilder()->CreateCall(function, arguments);
	if (functionType->getReturnType() != LLVMTypes::voidType)
	{
		callInstruction->setName(shortFunctionName + "_result");
	}
	return callInstruction;
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value* (LLVMCompileTimeContext*)> codeGenIfNotNull, llvm::Type* resultType, LLVMCompileTimeContext* context)
{
	auto codeGenIfNull = [=](LLVMCompileTimeContext* context)
	{
		return createZeroInitialisedConstant(resultType);
	};
	return createNullCheckSelect(valueToCheck, codeGenIfNotNull, codeGenIfNull, context);
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value* (LLVMCompileTimeContext*)> codeGenIfNotNull, llvm::PointerType* resultType, LLVMCompileTimeContext* context)
{
	return createNullCheckSelect(valueToCheck, codeGenIfNotNull, static_cast<llvm::Type*>(resultType), context);
}


llvm::Value* jitcat::LLVM::LLVMCodeGeneratorHelper::createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value* (LLVMCompileTimeContext*)> codeGenIfNotNull, std::function<llvm::Value* (LLVMCompileTimeContext*)> codeGenIfNull, LLVMCompileTimeContext* context)
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
	if (thenResult != nullptr && thenResult->getType() != LLVMTypes::voidType)
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


llvm::Value* LLVMCodeGeneratorHelper::createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, 
																	llvm::Type* resultType, LLVMCompileTimeContext* context)
{
	if (context->options.enableDereferenceNullChecks)
	{
		return createNullCheckSelect(valueToCheck, codeGenIfNotNull, resultType, context);
	}
	else
	{
		return codeGenIfNotNull(context);
	}
}


llvm::Value* LLVMCodeGeneratorHelper::createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value * (LLVMCompileTimeContext*)> codeGenIfNotNull,
																				  llvm::PointerType* resultType, LLVMCompileTimeContext* context)
{
	return createOptionalNullCheckSelect(valueToCheck, codeGenIfNotNull, static_cast<llvm::Type*>(resultType), context);
}


llvm::Value* LLVMCodeGeneratorHelper::createOptionalNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, 
																	std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNull, LLVMCompileTimeContext* context)
{
	if (context->options.enableDereferenceNullChecks)
	{
		return createNullCheckSelect(valueToCheck, codeGenIfNotNull, codeGenIfNull, context);
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
	else if (type.isCharType())							return LLVMTypes::charType;
	else if (type.isUCharType())						return LLVMTypes::charType;
	else if (type.isIntType())							return LLVMTypes::intType;
	else if (type.isUIntType())							return LLVMTypes::intType;
	else if (type.isInt64Type())						return LLVMTypes::longintType;
	else if (type.isUInt64Type())						return LLVMTypes::longintType;
	else if (type.isBoolType())							return LLVMTypes::boolType;
	else if (type.isReflectableHandleType())			return LLVMTypes::pointerType;
	else if (type.isVoidType())							return LLVMTypes::voidType;
	else if (type.isEnumType())							return toLLVMType(type.getUnderlyingEnumType());
	else if (type.isReflectableObjectType())			
	{
		//This is a compound type. For now, just create a pointer type.
		return LLVMTypes::pointerType;
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


llvm::Value* LLVMCodeGeneratorHelper::convertType(llvm::Value* valueToConvert, const CatGenericType& fromType, const CatGenericType& toType, 
																LLVMCompileTimeContext* context)
{
	assert(toLLVMType(fromType) == valueToConvert->getType());
	if (fromType.compare(toType, false, true))
	{
		return valueToConvert;
	}
	else if (fromType.isBasicType() && toType.isBasicType())
	{
		return convertType(valueToConvert, fromType.isSignedType(), toLLVMType(toType), toType.isSignedType(), context);
	}
	else if (toType.isStringType())
	{
		return convertToString(valueToConvert, fromType, context);
	}
	else if (fromType.isStringType())
	{
		if (toType.isBoolType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToBoolean, {valueToConvert}, "stringToBoolean");
		}
		else if (toType.isCharType())
		{
			return convertType(createIntrinsicCall(context, &LLVMCatIntrinsics::stringToInt, {valueToConvert}, "stringToInt"), true, toLLVMType(toType), true, context);
		}
		else if (toType.isUCharType())
		{
			return convertType(createIntrinsicCall(context, &LLVMCatIntrinsics::stringToUInt, {valueToConvert}, "stringToUInt"), false, toLLVMType(toType), false, context);
		}
		else if (toType.isIntType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToInt, {valueToConvert}, "stringToInt");
		}
		else if (toType.isUIntType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToUInt, {valueToConvert}, "stringToUInt");
		}
		else if (toType.isInt64Type())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToInt64, {valueToConvert}, "stringToInt64");
		}
		else if (toType.isUInt64Type())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::stringToUInt64, {valueToConvert}, "stringToUInt64");
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


llvm::Value* LLVMCodeGeneratorHelper::convertType(llvm::Value* valueToConvert, bool valueIsSigned, llvm::Type* toType, bool toIsSigned, LLVMCompileTimeContext* context)
{
	auto builder = codeGenerator->getBuilder();
	if (valueToConvert->getType() == toType && valueIsSigned == toIsSigned)
	{
		return valueToConvert;
	}
	if (toType == LLVMTypes::boolType)
	{
		//to 'boolean' type
		if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::charType
				|| valueToConvert->getType() == LLVMTypes::intType
				|| valueToConvert->getType() == LLVMTypes::longintType)
		{
			int bitWidth = static_cast<llvm::IntegerType*>( valueToConvert->getType())->getBitWidth();
			if (valueIsSigned)
			{
				llvm::Value* zero = llvm::ConstantInt::get(llvmContext, llvm::APInt(bitWidth, 0, true));
				return codeGenerator->booleanCast(builder->CreateICmpSGT(valueToConvert, zero, "GreaterThanZero"));
			}
			else
			{
				llvm::Value* zero = llvm::ConstantInt::get(llvmContext, llvm::APInt(bitWidth, 0, false));
				return codeGenerator->booleanCast(builder->CreateICmpUGT(valueToConvert, zero, "GreaterThanZero"));
			}
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			llvm::Value* zero = llvm::ConstantFP::get(llvmContext, llvm::APFloat(0.0f));
			return codeGenerator->booleanCast(builder->CreateFCmpUGT(valueToConvert, zero, "FGreaterThanZero"));
		}
		else if (valueToConvert->getType() == LLVMTypes::doubleType)
		{
			llvm::Value* zero = llvm::ConstantFP::get(llvmContext, llvm::APFloat(0.0));
			return codeGenerator->booleanCast(builder->CreateFCmpUGT(valueToConvert, zero, "FGreaterThanZero"));
		}
	}
	else if (toType == LLVMTypes::charType)
	{
		//to int type
		if (valueToConvert->getType() == LLVMTypes::charType)
		{
			//Sign conversion, no actual work is needed.
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::intType
				 || valueToConvert->getType() == LLVMTypes::longintType)
		{
			//Trunc works for both signed and unsigned values
			return builder->CreateTrunc(valueToConvert, LLVMTypes::charType, "TruncateToChar");
		}
		else if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return builder->CreateZExt(valueToConvert, LLVMTypes::charType, "ZeroExtended");
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			if (toIsSigned)
			{
				return builder->CreateFPToSI(valueToConvert, LLVMTypes::charType, "FloatToChar");
			}
			else
			{
				return builder->CreateFPToUI(valueToConvert, LLVMTypes::charType, "FloatToUChar");
			}
		}
		else if (valueToConvert->getType() == LLVMTypes::doubleType)
		{
			if (toIsSigned)
			{
				return builder->CreateFPToSI(valueToConvert, LLVMTypes::charType, "DoubleToChar");
			}
			else
			{
				return builder->CreateFPToUI(valueToConvert, LLVMTypes::charType, "DoubleToUChar");
			}
		}
	}
	else if (toType == LLVMTypes::intType)
	{
		//to int type
		if (valueToConvert->getType() == LLVMTypes::intType)
		{
			//Sign conversion, no actual work is needed.
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::charType)
		{
			//char to int
			if (toIsSigned && valueIsSigned)
			{
				return builder->CreateSExt(valueToConvert, LLVMTypes::intType, "SignExtended");
			}
			else
			{
				return builder->CreateZExt(valueToConvert, LLVMTypes::intType, "ZeroExtended");
			}
		}
		else if (valueToConvert->getType() == LLVMTypes::longintType)
		{
			//Trunc works for both signed and unsigned values
			return builder->CreateTrunc(valueToConvert, LLVMTypes::intType, "LongToInt");
		}
		else if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return builder->CreateZExt(valueToConvert, LLVMTypes::intType, "ZeroExtended");
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			if (toIsSigned)
			{
				return builder->CreateFPToSI(valueToConvert, LLVMTypes::intType, "FloatToInt");
			}
			else
			{
				return builder->CreateFPToUI(valueToConvert, LLVMTypes::intType, "FloatToUInt");
			}
		}
		else if (valueToConvert->getType() == LLVMTypes::doubleType)
		{
			if (toIsSigned)
			{
				return builder->CreateFPToSI(valueToConvert, LLVMTypes::intType, "DoubleToInt");
			}
			else
			{
				return builder->CreateFPToUI(valueToConvert, LLVMTypes::intType, "DoubleToInt");
			}
		}
	}
	else if (toType == LLVMTypes::longintType)
	{
		if (valueToConvert->getType() == LLVMTypes::longintType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == LLVMTypes::boolType)
		{
			return builder->CreateZExt(valueToConvert, LLVMTypes::longintType, "ZeroExtendedLong");
		}
		else if (valueToConvert->getType() == LLVMTypes::intType
				 || valueToConvert->getType() == LLVMTypes::charType)
		{
			if (valueIsSigned && toIsSigned)
			{
				return builder->CreateSExt(valueToConvert, LLVMTypes::longintType, "SignedToLong");
			}
			else
			{
				return builder->CreateZExt(valueToConvert, LLVMTypes::longintType, "UnsignedToLong");
			}
		}
		else if (valueToConvert->getType() == LLVMTypes::floatType)
		{
			return builder->CreateFPToSI(valueToConvert, LLVMTypes::longintType, "FloatToLong");
		}
		else if (valueToConvert->getType() == LLVMTypes::doubleType)
		{
			return builder->CreateFPToSI(valueToConvert, LLVMTypes::longintType, "DoubleToLong");
		}
	}
	else if (toType == LLVMTypes::floatType)
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
		else if (valueToConvert->getType() == LLVMTypes::charType
				 || valueToConvert->getType() == LLVMTypes::intType
				 || valueToConvert->getType() == LLVMTypes::longintType)
		{
			if (valueIsSigned)
			{
				return builder->CreateSIToFP(valueToConvert, LLVMTypes::floatType, "SignedToFloat");
			}
			else
			{
				return builder->CreateUIToFP(valueToConvert, LLVMTypes::floatType, "UnsignedToFloat");
			}
		}
	}
	else if (toType == LLVMTypes::doubleType)
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
		else if (valueToConvert->getType() == LLVMTypes::charType
				 || valueToConvert->getType() == LLVMTypes::intType
				 || valueToConvert->getType() == LLVMTypes::longintType)
		{
			if (valueIsSigned)
			{
				return builder->CreateSIToFP(valueToConvert, LLVMTypes::doubleType, "SignedToDouble");
			}
			else
			{
				return builder->CreateUIToFP(valueToConvert, LLVMTypes::doubleType, "UnsignedToDouble");
			}
		}
	}

	LLVMJit::logError("ERROR: Invalid type conversion.");
	assert(false);
	return nullptr;
}


llvm::Value* LLVMCodeGeneratorHelper::convertToString(llvm::Value* valueToConvert, const CatGenericType& fromType, LLVMCompileTimeContext* context)
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
	else if (valueToConvert->getType() == LLVMTypes::charType)
	{
		if (fromType.isSignedType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::intToString, {convertType(valueToConvert, true, LLVMTypes::intType, true, context)}, "intToString");
		}
		else
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::uIntToString, {convertType(valueToConvert, false, LLVMTypes::intType, false, context)}, "uIntToString");
		}
	}
	else if (valueToConvert->getType() == LLVMTypes::intType)
	{
		if (fromType.isSignedType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::intToString, {valueToConvert}, "intToString");
		}
		else
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::uIntToString, {valueToConvert}, "uIntToString");
		}
	}
	else if (valueToConvert->getType() == LLVMTypes::longintType)
	{
		if (fromType.isSignedType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::int64ToString, {valueToConvert}, "int64ToString");
		}
		else
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::uInt64ToString, {valueToConvert}, "uInt64ToString");
		}
	}
	else
	{
		return valueToConvert;
	}
}


llvm::Value* LLVMCodeGeneratorHelper::convertToPointer(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type)
{
	if (addressValue->getType() == type)
	{
		return addressValue;
	}
	llvm::Value* intToPtr =  codeGenerator->getBuilder()->CreateIntToPtr(addressValue, type);
	intToPtr->setName(name);
	return intToPtr;
}


llvm::Value* LLVMCodeGeneratorHelper::convertToPointer(llvm::Constant* addressConstant, const std::string& name, llvm::PointerType* type)
{
	return convertToPointer(static_cast<llvm::Value*>(addressConstant), name, type);
}


llvm::Value* LLVMCodeGeneratorHelper::convertToIntPtr(llvm::Value* llvmPointer, const std::string& name)
{
	llvm::Value* ptrToInt = codeGenerator->getBuilder()->CreatePtrToInt(llvmPointer, LLVMTypes::uintPtrType);
	ptrToInt->setName(name);
	return ptrToInt;
}


bool LLVMCodeGeneratorHelper::isPointer(llvm::Type* type)
{
	return type == LLVMTypes::pointerType;
}


bool LLVMCodeGeneratorHelper::isIntPtr(llvm::Type* type)
{
	return type == LLVMTypes::uintPtrType;
}


bool LLVMCodeGeneratorHelper::isInt(llvm::Type* type)
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


bool LLVMCodeGeneratorHelper::isInt(llvm::Value* value)
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

llvm::Value* LLVMCodeGeneratorHelper::loadBasicType(llvm::Type* type, llvm::Constant* addressValue, const std::string& name)
{
	return loadBasicType(type, static_cast<llvm::Value*>(addressValue), name);
}


llvm::Value* LLVMCodeGeneratorHelper::loadPointerAtAddress(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type)
{
	auto builder = codeGenerator->getBuilder();
	llvm::Value* addressAsPointer = nullptr;
	if (!addressValue->getType()->isPointerTy())
	{
		addressAsPointer = builder->CreateIntToPtr(addressValue, llvm::PointerType::getUnqual(type));
	}
	else if (addressValue->getType()->getPointerTo() == type)
	{
		addressAsPointer = addressValue;
	}
	else
	{
		addressAsPointer = builder->CreatePointerCast(addressValue, type->getPointerTo(), name);
	}
	addressAsPointer->setName(name + "_Ptr");
	llvm::LoadInst* load = builder->CreateLoad(addressAsPointer);
	load->setName(name);
	return load;
}


llvm::Value* LLVMCodeGeneratorHelper::loadPointerAtAddress(llvm::Constant* addressValue, const std::string& name, llvm::PointerType* type)
{
	return loadPointerAtAddress(static_cast<llvm::Value*>(addressValue), name, type);
}


llvm::Value* LLVMCodeGeneratorHelper::createAdd(llvm::Value* value1, llvm::Value* value2, const std::string& name)
{
	if (value1->getType()->isPointerTy() && value2->getType()->isIntegerTy())
	{
		llvm::PointerType* pointerType = static_cast<llvm::PointerType*>(value1->getType());
		if (pointerType->getElementType()->isIntegerTy(8))
		{
			return codeGenerator->getBuilder()->CreateGEP(value1, value2, name);
		}
	}
	return codeGenerator->getBuilder()->CreateAdd(value1, value2, name);
}


llvm::Value* LLVMCodeGeneratorHelper::createAdd(llvm::Value* value1, llvm::Constant* value2, const std::string& name)
{
	return codeGenerator->getBuilder()->CreateAdd(value1, value2, name);
}


llvm::Constant* LLVMCodeGeneratorHelper::createZeroInitialisedConstant(llvm::Type* type)
{
	if		(type == LLVMTypes::boolType)		return createConstant(false);
	else if (type == LLVMTypes::floatType)		return createConstant(0.0f);
	else if (type == LLVMTypes::doubleType)		return createConstant(0.0);
	else if (type == LLVMTypes::intType)		return createConstant(0);
	else if (type == LLVMTypes::longintType)	return createConstant((uint64_t)0);
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


llvm::Constant* LLVMCodeGeneratorHelper::createCharConstant(char constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(8, constant, true));
}


llvm::Constant* LLVMCodeGeneratorHelper::createUCharConstant(unsigned char constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(8, constant, false));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(char constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(8, constant, true));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(unsigned char constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(8, constant, false));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(int constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(32, constant, true));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(unsigned int constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(32, (uint64_t)constant, false));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(int64_t constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(64, (uint64_t)constant, true));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(uint64_t constant)
{
	return llvm::ConstantInt::get(llvmContext, llvm::APInt(64, (uint64_t)constant, false));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(double constant)
{
	return llvm::ConstantFP::get(llvmContext, llvm::APFloat(constant));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(float constant)
{
	return llvm::ConstantFP::get(llvmContext, llvm::APFloat(constant));
}


llvm::Constant* LLVMCodeGeneratorHelper::createConstant(bool constant)
{
	return constant ? createConstant((char)1) : createConstant((char)0);
}


llvm::Constant* LLVMCodeGeneratorHelper::createNullPtrConstant(llvm::PointerType* pointerType)
{
	return llvm::ConstantPointerNull::get(pointerType);
}


llvm::Constant* jitcat::LLVM::LLVMCodeGeneratorHelper::createZeroTerminatedStringConstant(const std::string& value)
{
    auto charType = LLVMTypes::charType;


    //Initialize chars vector
    std::vector<llvm::Constant *> chars(value.length());
    for(unsigned int i = 0; i < value.size(); i++) {
      chars[i] = llvm::ConstantInt::get(charType, value[i]);
    }

    //add a zero terminator too
    chars.push_back(llvm::ConstantInt::get(charType, 0));

    //Initialize the string from the characters
    llvm::ArrayType* stringType = llvm::ArrayType::get(charType, chars.size());

	llvm::ArrayRef ref = value;

    //Create the declaration statement
    auto globalDeclaration = (llvm::GlobalVariable*) codeGenerator->getCurrentModule()->getOrInsertGlobal(Tools::append(value, ".str"), stringType);
    globalDeclaration->setInitializer(llvm::ConstantArray::get(stringType, chars));
    globalDeclaration->setConstant(true);
    globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
    globalDeclaration->setUnnamedAddr (llvm::GlobalValue::UnnamedAddr::Global);

    //Return a cast to an i8*
    return llvm::ConstantExpr::getBitCast(globalDeclaration, charType->getPointerTo());
}


llvm::Value* LLVMCodeGeneratorHelper::createPtrConstant(unsigned long long address, const std::string& name, llvm::PointerType* pointerType)
{
	llvm::Constant* constant = createIntPtrConstant(address, Tools::append(name, "_IntPtr"));
	return convertToPointer(constant, Tools::append(name, "_Ptr"), pointerType); 
}


llvm::Constant* LLVMCodeGeneratorHelper::createZeroInitialisedArrayConstant(llvm::ArrayType* arrayType)
{
	uint64_t numElements = arrayType->getArrayNumElements();
	std::vector<llvm::Constant*> constants;
	for (std::size_t i = 0; i < numElements; ++i)
	{
		constants.push_back(createZeroInitialisedConstant(arrayType->getArrayElementType()));
	}
	return llvm::ConstantArray::get(arrayType, constants);
}


llvm::Value* LLVMCodeGeneratorHelper::constantToValue(llvm::Constant* constant) const
{
	return constant;
}


llvm::Value* LLVMCodeGeneratorHelper::createObjectAllocA(LLVMCompileTimeContext* context, const std::string& name, 
																	   const CatGenericType& objectType, bool generateDestructorCall)
{
	auto builder = codeGenerator->getBuilder();
	llvm::Type* llvmObjectType = llvm::ArrayType::get(LLVMTypes::charType, objectType.getTypeSize());
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


llvm::Value* LLVMCodeGeneratorHelper::generateFunctionCallReturnValueAllocation(const CatGenericType& returnType, const std::string& functionName, 
																							  LLVMCompileTimeContext* context)
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


llvm::Value* LLVMCodeGeneratorHelper::generateStaticFunctionCall(const jitcat::CatGenericType& returnType, const std::vector<llvm::Value*>& argumentList, 
																			   const std::vector<llvm::Type*>& argumentTypes, LLVMCompileTimeContext* context, 
																			   const std::string& mangledFunctionName, const std::string& shortFunctionName,
																			   llvm::Value* returnedObjectAllocation)
{
	if (returnType.isReflectableObjectType())
	{
		llvm::FunctionType* functionType = llvm::FunctionType::get(LLVMTypes::voidType, argumentTypes, false);
		llvm::CallInst* call = static_cast<llvm::CallInst*>(createCall(functionType, argumentList, false, mangledFunctionName, shortFunctionName));
		call->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
		call->addDereferenceableAttr(1 , returnType.getTypeSize());
		return returnedObjectAllocation;
	}
	else
	{
		llvm::Type* returnLLVMType = toLLVMType(returnType);
		llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
		return createCall(functionType, argumentList, false, mangledFunctionName, shortFunctionName);
	}
}


llvm::Value* LLVMCodeGeneratorHelper::generateMemberFunctionCall(Reflection::MemberFunctionInfo* memberFunction, const AST::CatTypedExpression* base, 
																const std::vector<const AST::CatTypedExpression*>& arguments, 
																LLVMCompileTimeContext* context)
{
	const CatGenericType& returnType = memberFunction->getReturnType();
	llvm::Value* baseObject = codeGenerator->generate(base, context);
	if (memberFunction->isDeferredFunctionCall())
	{
		//We must first get the deferred base.
		DeferredMemberFunctionInfo* deferredFunctionInfo = static_cast<DeferredMemberFunctionInfo*>(memberFunction);
		baseObject = deferredFunctionInfo->getBaseMember()->generateDereferenceCode(baseObject, context);
	}
	//If the member function call returns an object by value, we must allocate the object on the stack.
	llvm::Value* returnedObjectAllocation = generateFunctionCallReturnValueAllocation(returnType, memberFunction->getMemberFunctionName(), context);
	
	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		MemberFunctionCallData callData = memberFunction->getFunctionAddress();
		assert(callData.functionAddress != 0 || callData.linkDylib || callData.inlineFunctionGenerator != nullptr);
		std::vector<llvm::Value*> argumentList;
		llvm::Value* functionThis = compileContext->helper->convertToPointer(baseObject, memberFunction->getMemberFunctionName() + "_This_Ptr");
		argumentList.push_back(functionThis);
		std::vector<llvm::Type*> argumentTypes;
		argumentTypes.push_back(LLVMTypes::pointerType);
		if (callData.callType == MemberFunctionCallType::ThisCallThroughStaticFunction)
		{
			//Add an argument that contains a pointer to a MemberFunctionInfo object.
			llvm::Value* memberFunctionAddressValue = compileContext->helper->createIntPtrConstant(callData.functionInfoStructAddress, "MemberFunctionInfo_IntPtr");
			llvm::Value* memberFunctionPtrValue = compileContext->helper->convertToPointer(memberFunctionAddressValue, "MemberFunctionInfo_Ptr");
			argumentList.push_back(memberFunctionPtrValue);
			argumentTypes.push_back(LLVMTypes::pointerType);
		}
		generateFunctionCallArgumentEvalatuation(arguments, memberFunction->getArgumentTypes(), argumentList, argumentTypes, codeGenerator, context);

		if (callData.callType != MemberFunctionCallType::InlineFunctionGenerator)
		{
			if (!callData.linkDylib)
			{
				defineWeakSymbol(callData.functionAddress, memberFunction->getMangledName());
			}
			else
			{
				//Make sure the dylib that contains the symbol is added to the search order.
				TypeInfo* typeInfo = base->getType().removeIndirection().getObjectType();
				if (typeInfo->isCustomType())
				{
					llvm::orc::JITDylib* functionLib = static_cast<CustomTypeInfo*>(typeInfo)->getDylib();
					assert(functionLib != nullptr);
					if (functionLib != codeGenerator->dylib && codeGenerator->linkedLibs.find(functionLib) == codeGenerator->linkedLibs.end())
					{
						codeGenerator->dylib->addToSearchOrder(*functionLib);
						codeGenerator->linkedLibs.insert(functionLib);
					}
				}
			}
		
			llvm::Type* returnLLVMType = context->helper->toLLVMType(returnType);
			if (!returnType.isReflectableObjectType())
			{
				if (callData.callType == MemberFunctionCallType::PseudoMemberCall)
				{
					return generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, memberFunction->getMangledName(), memberFunction->getMemberFunctionName(), returnedObjectAllocation);
				}
				else
				{
					llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
					llvm::CallInst* call = static_cast<llvm::CallInst*>(compileContext->helper->createCall(functionType, argumentList, 
																										   callData.callType == MemberFunctionCallType::ThisCall, 
																										   memberFunction->getMangledName(), 
																										   memberFunction->getMemberFunctionName()));
					if (Configuration::useThisCall && callData.callType == MemberFunctionCallType::ThisCall)
					{
						call->setCallingConv(llvm::CallingConv::X86_ThisCall);
					}
					return static_cast<llvm::Value*>(call);
				}
			}
			else if (callData.callType == MemberFunctionCallType::PseudoMemberCall)
			{
				argumentTypes.insert(argumentTypes.begin(), LLVMTypes::pointerType);
				argumentList.insert(argumentList.begin(), returnedObjectAllocation);
				return generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, memberFunction->getMangledName(), 
												  memberFunction->getMemberFunctionName(), returnedObjectAllocation);
			}
			else if (returnType.isReflectableObjectType())
			{
				auto sretTypeInsertPoint = argumentTypes.begin();
				if (!Configuration::sretBeforeThis && callData.callType == MemberFunctionCallType::ThisCall)
				{
					sretTypeInsertPoint++;
				}
				argumentTypes.insert(sretTypeInsertPoint, LLVMTypes::pointerType);

				llvm::FunctionType* functionType = llvm::FunctionType::get(LLVMTypes::voidType, argumentTypes, false);
				auto sretInsertPoint = argumentList.begin();
				if (!Configuration::sretBeforeThis && callData.callType == MemberFunctionCallType::ThisCall)
				{
					sretInsertPoint++;
				}
				argumentList.insert(sretInsertPoint, returnedObjectAllocation);
				llvm::CallInst* call = static_cast<llvm::CallInst*>(compileContext->helper->createCall(functionType, argumentList, 
																									   callData.callType == MemberFunctionCallType::ThisCall, 
																									   memberFunction->getMangledName(), 
																									   memberFunction->getMemberFunctionName()));
				if (Configuration::useThisCall && callData.callType == MemberFunctionCallType::ThisCall)
				{
					call->setCallingConv(llvm::CallingConv::X86_ThisCall);
				}
				call->addParamAttr(Configuration::sretBeforeThis ? 0 : 1, llvm::Attribute::AttrKind::StructRet);
				call->addDereferenceableAttr(Configuration::sretBeforeThis ? 1 : 2, returnType.getTypeSize());
				return returnedObjectAllocation;
			}
			else
			{
				return LLVMJit::logError("ERROR: Not yet supported.");
			}
		}
		else
		{
			assert(callData.inlineFunctionGenerator != nullptr);
			return (*callData.inlineFunctionGenerator)(context, argumentList);
		}
	};

	llvm::Type* resultType = toLLVMType(returnType);
	if (returnType.isReflectableObjectType())
	{
		resultType = resultType->getPointerTo();
	}
	if (returnType.isReflectableObjectType())
	{
		auto codeGenIfNull = [=](LLVMCompileTimeContext* context)
		{
			assert(returnType.isConstructible());
			llvm::Constant* typeInfoConstant = createIntPtrConstant(reinterpret_cast<uintptr_t>(returnType.getObjectType()), Tools::append(returnType.getObjectType()->getTypeName(), "_typeInfo"));
			llvm::Value* typeInfoConstantAsIntPtr = convertToPointer(typeInfoConstant, Tools::append(returnType.getObjectType()->getTypeName(), "_typeInfoPtr"));
			createIntrinsicCall(context, &LLVMCatIntrinsics::placementConstructType, {returnedObjectAllocation, typeInfoConstantAsIntPtr}, "placementConstructType");
			return returnedObjectAllocation;
		};
		return createOptionalNullCheckSelect(baseObject, notNullCodeGen, codeGenIfNull, context);
	}
	else
	{
		return createOptionalNullCheckSelect(baseObject, notNullCodeGen, resultType, context);
	}
}


void LLVMCodeGeneratorHelper::generateLoop(LLVMCompileTimeContext* context, 
										   llvm::Value* iteratorBeginValue,
										   llvm::Value* iteratorStepValue,
										   llvm::Value* iteratorEndValue,
										   const std::function<void (LLVMCompileTimeContext*, llvm::Value*)>& generateLoopBody)
{
	assert(false);
	assert(iteratorBeginValue->getType()->isIntegerTy());
	assert(iteratorStepValue->getType()->isIntegerTy());
	assert(iteratorEndValue->getType()->isIntegerTy());


	auto helper = context->helper;
	auto builder = helper->getBuilder();
	llvm::Value* iteratorAlloc = builder->CreateAlloca(iteratorBeginValue->getType(), nullptr, "iteratorAlloc"); 
	builder->CreateStore(iteratorBeginValue, iteratorAlloc);
	llvm::BasicBlock* preheaderBlock = builder->GetInsertBlock();

	//Create the basic blocks for the for-loop
	llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(helper->getContext(), "loopCondition", context->currentFunction);
	llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(helper->getContext(), "loopBody");
	llvm::BasicBlock* continueBlock = continueBlock = llvm::BasicBlock::Create(helper->getContext(), "continue");

	//Insert an explicit fall through from the current block to the conditionBlock.
	builder->CreateBr(conditionBlock);
	//Start insertion in conditionBlock.
	builder->SetInsertPoint(conditionBlock);

	// Check the loop condition
	llvm::Value* iterator = builder->CreateLoad(iteratorAlloc, "iteratorLoad");
	llvm::Value* condition = builder->CreateICmpSLT(iterator, iteratorEndValue, "loopConditionCheck");
	
	//Jump to either the loop block, or the continuation block.
	builder->CreateCondBr(condition, loopBlock, continueBlock);

	context->currentFunction->getBasicBlockList().push_back(loopBlock);

	// Start insertion in loopBlock.
	builder->SetInsertPoint(loopBlock);
	
	//Generate loop body
	generateLoopBody(context, iterator);
	//Increment iterator
	builder->CreateStore(iteratorAlloc, builder->CreateAdd(iterator, iteratorStepValue, "incrementIterator"));
	//Jump back to the condition
	builder->CreateBr(conditionBlock);
	context->currentFunction->getBasicBlockList().push_back(continueBlock);
	//Continue insertion in continueBlock.
	builder->SetInsertPoint(continueBlock);
}


void LLVMCodeGeneratorHelper::defineWeakSymbol(intptr_t functionAddress, const std::string& mangledFunctionName)
{
	//Define the function in the runtime library.
	llvm::JITSymbolFlags functionFlags;
	functionFlags |= llvm::JITSymbolFlags::Callable;
	functionFlags |= llvm::JITSymbolFlags::Exported;
	functionFlags |= llvm::JITSymbolFlags::Absolute;
	functionFlags |= llvm::JITSymbolFlags::Weak;
	llvm::orc::SymbolMap intrinsicSymbols;
	llvm::JITTargetAddress address = functionAddress;
	intrinsicSymbols[codeGenerator->executionSession->intern(mangledFunctionName)] = llvm::JITEvaluatedSymbol(address, functionFlags);
	llvm::cantFail(codeGenerator->runtimeLibraryDyLib->define(llvm::orc::absoluteSymbols(intrinsicSymbols)));
}


llvm::Value* LLVMCodeGeneratorHelper::convertIndirection(llvm::Value* value, llvm::Type* expectedType)
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

llvm::Value* LLVMCodeGeneratorHelper::copyConstructIfValueType(llvm::Value* value, const CatGenericType& type, LLVMCompileTimeContext* context, const std::string& valueName)
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


llvm::Value* LLVMCodeGeneratorHelper::generateIntrinsicCall(jitcat::Reflection::StaticFunctionInfo* functionInfo, std::vector<llvm::Value*>& arguments, LLVMCompileTimeContext* context)
{
	//Define the function in the runtime library.
	defineWeakSymbol(functionInfo->getFunctionAddress(), functionInfo->getNormalFunctionName());
	
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

	return generateStaticFunctionCall(functionInfo->getReturnType(), arguments, argumentLLVMTypes, context, functionInfo->getNormalFunctionName(), functionInfo->getNormalFunctionName(),  returnValueAllocation);
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