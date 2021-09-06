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
#include "jitcat/LLVMPrecompilationContext.h"
#include "jitcat/LLVMTargetConfig.h"
#include "jitcat/LLVMTargetConfigOptions.h"
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
	llvmTypes(codeGenerator->targetConfig->getLLVMTypes()),
	llvmContext(LLVMJit::get().getContext())
{
}


llvm::Value* LLVMCodeGeneratorHelper::createCall(LLVMCompileTimeContext* context, llvm::FunctionType* functionType, const std::vector<llvm::Value*>& arguments, bool isThisCall, 
												 const std::string& mangledFunctionName, const std::string& shortFunctionName, bool isDirectlyLinked)
{
	llvm::CallInst* callInstruction = nullptr;
	if (context->isPrecompilationContext && !isDirectlyLinked)
	{
		//When pre-compiling, most functions will be called through a function pointer that's stored in a GlobalVariable.
		//The only exceptions are the JitCat std-lib functions that are linked directly through extern "C" functions.

		//this global variable is a pointer to an int-ptr.
		llvm::GlobalVariable* functionPointerPointer = std::static_pointer_cast<LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalFunctionPointer(mangledFunctionName, context);
		auto builder = getBuilder();
		llvm::Value* functionPointer = loadPointerAtAddress(functionPointerPointer, "FunctionPointer", functionType->getPointerTo());
		llvm::FunctionCallee callee(functionType, functionPointer);
		callInstruction = builder->CreateCall(callee, arguments);
	}
	else
	{
		//First try and find the function in the current module.
		llvm::Function* function = codeGenerator->getCurrentModule()->getFunction(mangledFunctionName);
		if (function == nullptr)
		{
			//If the function was not found, create the function signature and add it to the module. 
			function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, mangledFunctionName.c_str(), codeGenerator->getCurrentModule());
			if (codeGenerator->targetConfig->useThisCall && isThisCall)
			{
				function->setCallingConv(llvm::CallingConv::X86_ThisCall);
			}
			else
			{
				function->setCallingConv(context->targetConfig->getOptions().defaultLLVMCallingConvention);
			}
		}
		else
		{
			//Check if the function has the correct signature
			assert(function->getFunctionType() == functionType);
		}
		callInstruction = getBuilder()->CreateCall(function, arguments);
	}
	if (functionType->getReturnType() != llvmTypes.voidType)
	{
		callInstruction->setName(shortFunctionName + "_result");
	}
	callInstruction->setCallingConv(context->targetConfig->getOptions().defaultLLVMCallingConvention);
	return callInstruction;
}


llvm::Value* LLVMCodeGeneratorHelper::createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value*(LLVMCompileTimeContext*)> codeGenIfNotNull, 
															const CatGenericType& returnType, llvm::Value* returnedObjectAllocation, LLVMCompileTimeContext* context)
{
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
			llvm::Value* typeInfoConstantAsIntPtr = createTypeInfoGlobalValue(context, returnType.getObjectType());
			createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementConstructType, {returnedObjectAllocation, typeInfoConstantAsIntPtr}, "_jc_placementConstructType", true);
			return returnedObjectAllocation;
		};
		return createNullCheckSelect(valueToCheck, codeGenIfNotNull, codeGenIfNull, context);
	}
	else
	{
		return createNullCheckSelect(valueToCheck, codeGenIfNotNull, resultType, context);
	}
}


llvm::Value* LLVMCodeGeneratorHelper::createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value* (LLVMCompileTimeContext*)> codeGenIfNotNull, llvm::Type* resultType, LLVMCompileTimeContext* context)
{
	auto codeGenIfNull = [=](LLVMCompileTimeContext* context)
	{
		return createZeroInitialisedConstant(resultType);
	};
	return createNullCheckSelect(valueToCheck, codeGenIfNotNull, codeGenIfNull, context);
}


llvm::Value* LLVMCodeGeneratorHelper::createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value* (LLVMCompileTimeContext*)> codeGenIfNotNull, llvm::PointerType* resultType, LLVMCompileTimeContext* context)
{
	return createNullCheckSelect(valueToCheck, codeGenIfNotNull, static_cast<llvm::Type*>(resultType), context);
}


llvm::Value* LLVMCodeGeneratorHelper::createNullCheckSelect(llvm::Value* valueToCheck, std::function<llvm::Value* (LLVMCompileTimeContext*)> codeGenIfNotNull, std::function<llvm::Value* (LLVMCompileTimeContext*)> codeGenIfNull, LLVMCompileTimeContext* context)
{
	auto builder = codeGenerator->getBuilder();
	llvm::Value* isNotNull = nullptr;
	if (valueToCheck->getType() != llvmTypes.boolType)
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
	if (thenResult != nullptr && thenResult->getType() != llvmTypes.voidType)
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
																	const CatGenericType& returnType, llvm::Value* returnedObjectAllocation, LLVMCompileTimeContext* context)
{
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
			llvm::Value* typeInfoConstantAsIntPtr = createTypeInfoGlobalValue(context, returnType.getObjectType());
			createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementConstructType, {returnedObjectAllocation, typeInfoConstantAsIntPtr}, "_jc_placementConstructType", true);
			return returnedObjectAllocation;
		};
		return createOptionalNullCheckSelect(valueToCheck, codeGenIfNotNull, codeGenIfNull, context);
	}
	else
	{
		return createOptionalNullCheckSelect(valueToCheck, codeGenIfNotNull, resultType, context);
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


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(unsigned int intrinsic, const CatGenericType& parameterType, 
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


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(unsigned int intrinsic, const CatGenericType& overload1Type, 
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


llvm::Value* LLVMCodeGeneratorHelper::callIntrinsic(unsigned int intrinsic, const CatGenericType& parameter1Type, const CatGenericType& parameter2Type, 
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
	if		(type.isFloatType())						return llvmTypes.floatType;
	else if	(type.isDoubleType())						return llvmTypes.doubleType;
	else if (type.isCharType())							return llvmTypes.charType;
	else if (type.isUCharType())						return llvmTypes.charType;
	else if (type.isIntType())							return llvmTypes.intType;
	else if (type.isUIntType())							return llvmTypes.intType;
	else if (type.isInt64Type())						return llvmTypes.longintType;
	else if (type.isUInt64Type())						return llvmTypes.longintType;
	else if (type.isBoolType())							return llvmTypes.boolType;
	else if (type.isReflectableHandleType())			return llvmTypes.pointerType;
	else if (type.isVoidType())							return llvmTypes.voidType;
	else if (type.isEnumType())							return toLLVMType(type.getUnderlyingEnumType());
	else if (type.isReflectableObjectType())			
	{
		//This is a compound type. For now, just create a pointer type.
		return llvmTypes.pointerType;
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
			return llvmTypes.pointerType;
		}

	}
	else
	{
		//Unknown type. Add it to this function.
		assert(false);
		return llvmTypes.voidType;
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


llvm::Function* LLVMCodeGeneratorHelper::generateGlobalVariableEnumerationFunction(const std::unordered_map<std::string, llvm::GlobalVariable*>& globals, 
																				   const std::string& functionName)
{
	llvm::Type* functionReturnType = llvmTypes.voidType;

	std::vector<llvm::Type*> callBackParameters = {llvmTypes.pointerType, llvmTypes.pointerType};
	llvm::FunctionType* callBackType = llvm::FunctionType::get(functionReturnType, callBackParameters, false);		

	std::vector<llvm::Type*> parameters = {callBackType->getPointerTo()};
	llvm::FunctionType* functionType = llvm::FunctionType::get(functionReturnType, parameters, false);		
	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, functionName.c_str(), codeGenerator->getCurrentModule());
	function->setCallingConv(codeGenerator->targetConfig->getOptions().defaultLLVMCallingConvention);

	llvm::FunctionCallee callee(callBackType, function->getArg(0));

	llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
	auto builder = getBuilder();
	builder->SetInsertPoint(&function->getEntryBlock());

	for (auto iter : globals)
	{
		llvm::Constant* zeroTerminatedString = createZeroTerminatedStringConstant(iter.first);
		llvm::Value* globalPtr = convertToPointer(iter.second, "globalPtr");
		llvm::CallInst* callInst = builder->CreateCall(callee, {zeroTerminatedString, globalPtr});
		callInst->setCallingConv(codeGenerator->targetConfig->getOptions().defaultLLVMCallingConvention);
	}
	builder->CreateRetVoid();
	return codeGenerator->verifyAndOptimizeFunction(function);
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
			return createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToBoolean, {valueToConvert}, "_jc_stringToBoolean", true);
		}
		else if (toType.isCharType())
		{
			return convertType(createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToInt, {valueToConvert}, "_jc_stringToInt", true), true, toLLVMType(toType), true, context);
		}
		else if (toType.isUCharType())
		{
			return convertType(createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToUInt, {valueToConvert}, "_jc_stringToUInt", true), false, toLLVMType(toType), false, context);
		}
		else if (toType.isIntType())
		{
			return createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToInt, {valueToConvert}, "_jc_stringToInt", true);
		}
		else if (toType.isUIntType())
		{
			return createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToUInt, {valueToConvert}, "_jc_stringToUInt", true);
		}
		else if (toType.isInt64Type())
		{
			return createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToInt64, {valueToConvert}, "_jc_stringToInt64", true);
		}
		else if (toType.isUInt64Type())
		{
			return createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToUInt64, {valueToConvert}, "_jc_stringToUInt64", true);
		}
		else if (toType.isFloatType())
		{
			return createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToFloat, {valueToConvert}, "_jc_stringToFloat", true);
		}
		else if (toType.isDoubleType())
		{
			return createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_stringToDouble, {valueToConvert}, "_jc_stringToDouble", true);
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
	if (toType == llvmTypes.boolType)
	{
		//to 'boolean' type
		if (valueToConvert->getType() == llvmTypes.boolType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == llvmTypes.charType
				|| valueToConvert->getType() == llvmTypes.intType
				|| valueToConvert->getType() == llvmTypes.longintType)
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
		else if (valueToConvert->getType() == llvmTypes.floatType)
		{
			llvm::Value* zero = llvm::ConstantFP::get(llvmContext, llvm::APFloat(0.0f));
			return codeGenerator->booleanCast(builder->CreateFCmpUGT(valueToConvert, zero, "FGreaterThanZero"));
		}
		else if (valueToConvert->getType() == llvmTypes.doubleType)
		{
			llvm::Value* zero = llvm::ConstantFP::get(llvmContext, llvm::APFloat(0.0));
			return codeGenerator->booleanCast(builder->CreateFCmpUGT(valueToConvert, zero, "FGreaterThanZero"));
		}
	}
	else if (toType == llvmTypes.charType)
	{
		//to int type
		if (valueToConvert->getType() == llvmTypes.charType)
		{
			//Sign conversion, no actual work is needed.
			return valueToConvert;
		}
		else if (valueToConvert->getType() == llvmTypes.intType
				 || valueToConvert->getType() == llvmTypes.longintType)
		{
			//Trunc works for both signed and unsigned values
			return builder->CreateTrunc(valueToConvert, llvmTypes.charType, "TruncateToChar");
		}
		else if (valueToConvert->getType() == llvmTypes.boolType)
		{
			return builder->CreateZExt(valueToConvert, llvmTypes.charType, "ZeroExtended");
		}
		else if (valueToConvert->getType() == llvmTypes.floatType)
		{
			if (toIsSigned)
			{
				return builder->CreateFPToSI(valueToConvert, llvmTypes.charType, "FloatToChar");
			}
			else
			{
				return builder->CreateFPToUI(valueToConvert, llvmTypes.charType, "FloatToUChar");
			}
		}
		else if (valueToConvert->getType() == llvmTypes.doubleType)
		{
			if (toIsSigned)
			{
				return builder->CreateFPToSI(valueToConvert, llvmTypes.charType, "DoubleToChar");
			}
			else
			{
				return builder->CreateFPToUI(valueToConvert, llvmTypes.charType, "DoubleToUChar");
			}
		}
	}
	else if (toType == llvmTypes.intType)
	{
		//to int type
		if (valueToConvert->getType() == llvmTypes.intType)
		{
			//Sign conversion, no actual work is needed.
			return valueToConvert;
		}
		else if (valueToConvert->getType() == llvmTypes.charType)
		{
			//char to int
			if (toIsSigned && valueIsSigned)
			{
				return builder->CreateSExt(valueToConvert, llvmTypes.intType, "SignExtended");
			}
			else
			{
				return builder->CreateZExt(valueToConvert, llvmTypes.intType, "ZeroExtended");
			}
		}
		else if (valueToConvert->getType() == llvmTypes.longintType)
		{
			//Trunc works for both signed and unsigned values
			return builder->CreateTrunc(valueToConvert, llvmTypes.intType, "LongToInt");
		}
		else if (valueToConvert->getType() == llvmTypes.boolType)
		{
			return builder->CreateZExt(valueToConvert, llvmTypes.intType, "ZeroExtended");
		}
		else if (valueToConvert->getType() == llvmTypes.floatType)
		{
			if (toIsSigned)
			{
				return builder->CreateFPToSI(valueToConvert, llvmTypes.intType, "FloatToInt");
			}
			else
			{
				return builder->CreateFPToUI(valueToConvert, llvmTypes.intType, "FloatToUInt");
			}
		}
		else if (valueToConvert->getType() == llvmTypes.doubleType)
		{
			if (toIsSigned)
			{
				return builder->CreateFPToSI(valueToConvert, llvmTypes.intType, "DoubleToInt");
			}
			else
			{
				return builder->CreateFPToUI(valueToConvert, llvmTypes.intType, "DoubleToInt");
			}
		}
	}
	else if (toType == llvmTypes.longintType)
	{
		if (valueToConvert->getType() == llvmTypes.longintType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == llvmTypes.boolType)
		{
			return builder->CreateZExt(valueToConvert, llvmTypes.longintType, "ZeroExtendedLong");
		}
		else if (valueToConvert->getType() == llvmTypes.intType
				 || valueToConvert->getType() == llvmTypes.charType)
		{
			if (valueIsSigned && toIsSigned)
			{
				return builder->CreateSExt(valueToConvert, llvmTypes.longintType, "SignedToLong");
			}
			else
			{
				return builder->CreateZExt(valueToConvert, llvmTypes.longintType, "UnsignedToLong");
			}
		}
		else if (valueToConvert->getType() == llvmTypes.floatType)
		{
			return builder->CreateFPToSI(valueToConvert, llvmTypes.longintType, "FloatToLong");
		}
		else if (valueToConvert->getType() == llvmTypes.doubleType)
		{
			return builder->CreateFPToSI(valueToConvert, llvmTypes.longintType, "DoubleToLong");
		}
	}
	else if (toType == llvmTypes.floatType)
	{
		//to float type
		if (valueToConvert->getType() == llvmTypes.floatType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == llvmTypes.doubleType)
		{
			return builder->CreateFPCast(valueToConvert, llvmTypes.floatType, "DoubleToFloat");
		}
		else if (valueToConvert->getType() == llvmTypes.boolType)
		{
			return builder->CreateUIToFP(valueToConvert, llvmTypes.floatType, "BoolToFloat");
		}
		else if (valueToConvert->getType() == llvmTypes.charType
				 || valueToConvert->getType() == llvmTypes.intType
				 || valueToConvert->getType() == llvmTypes.longintType)
		{
			if (valueIsSigned)
			{
				return builder->CreateSIToFP(valueToConvert, llvmTypes.floatType, "SignedToFloat");
			}
			else
			{
				return builder->CreateUIToFP(valueToConvert, llvmTypes.floatType, "UnsignedToFloat");
			}
		}
	}
	else if (toType == llvmTypes.doubleType)
	{
		//to float type
		if (valueToConvert->getType() == llvmTypes.doubleType)
		{
			return valueToConvert;
		}
		else if (valueToConvert->getType() == llvmTypes.floatType)
		{
			return builder->CreateFPCast(valueToConvert, llvmTypes.doubleType, "FloatToDouble");
		}
		else if (valueToConvert->getType() == llvmTypes.boolType)
		{
			return builder->CreateUIToFP(valueToConvert, llvmTypes.doubleType, "BoolToDouble");
		}
		else if (valueToConvert->getType() == llvmTypes.charType
				 || valueToConvert->getType() == llvmTypes.intType
				 || valueToConvert->getType() == llvmTypes.longintType)
		{
			if (valueIsSigned)
			{
				return builder->CreateSIToFP(valueToConvert, llvmTypes.doubleType, "SignedToDouble");
			}
			else
			{
				return builder->CreateUIToFP(valueToConvert, llvmTypes.doubleType, "UnsignedToDouble");
			}
		}
	}

	LLVMJit::logError("ERROR: Invalid type conversion.");
	assert(false);
	return nullptr;
}


llvm::Value* LLVMCodeGeneratorHelper::generateStaticPointerVariable(uintptr_t valueWhenJitCompiling, LLVMCompileTimeContext* context, const std::string& name)
{
	if (!context->isPrecompilationContext)
	{
		return context->helper->createPtrConstant(context, valueWhenJitCompiling, "pointerTo_" + name);
	}
	else
	{
		llvm::GlobalVariable* globalVariable = std::static_pointer_cast<LLVM::LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalVariable(name, context);
		llvm::Value* loadedPointer = context->helper->loadPointerAtAddress(globalVariable, name, llvmTypes.pointerType);
		llvm::MDNode* metaData = llvm::MDNode::get(LLVMJit::get().getContext(), llvm::None);
		static_cast<llvm::LoadInst*>(loadedPointer)->setMetadata(llvm::LLVMContext::MD_nonnull, metaData);
		return loadedPointer;
	}
}


llvm::Function* jitcat::LLVM::LLVMCodeGeneratorHelper::generateConstIntFunction(int value, const std::string& name)
{
	llvm::FunctionType* functionType = llvm::FunctionType::get(llvmTypes.intType, {}, false);		
	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, name.c_str(), codeGenerator->getCurrentModule());
	function->setCallingConv(codeGenerator->targetConfig->getOptions().defaultLLVMCallingConvention);

	llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
	auto builder = getBuilder();
	builder->SetInsertPoint(&function->getEntryBlock());
	builder->CreateRet(createConstant(value));
	return codeGenerator->verifyAndOptimizeFunction(function);
}


llvm::Value* LLVMCodeGeneratorHelper::convertToString(llvm::Value* valueToConvert, const CatGenericType& fromType, LLVMCompileTimeContext* context)
{
	if (valueToConvert->getType() == llvmTypes.boolType)
	{
		return createIntrinsicCall(context, &LLVMCatIntrinsics::boolToString, {valueToConvert}, "boolToString", false);
	}
	else if (valueToConvert->getType() == llvmTypes.doubleType)
	{
		return createIntrinsicCall(context, &LLVMCatIntrinsics::doubleToString, {valueToConvert}, "doubleToString", false);
	}
	else if (valueToConvert->getType() == llvmTypes.floatType)
	{
		return createIntrinsicCall(context, &LLVMCatIntrinsics::floatToString, {valueToConvert}, "floatToString", false);
	}
	else if (valueToConvert->getType() == llvmTypes.charType)
	{
		if (fromType.isSignedType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::intToString, {convertType(valueToConvert, true, llvmTypes.intType, true, context)}, "intToString", false);
		}
		else
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::uIntToString, {convertType(valueToConvert, false, llvmTypes.intType, false, context)}, "uIntToString", false);
		}
	}
	else if (valueToConvert->getType() == llvmTypes.intType)
	{
		if (fromType.isSignedType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::intToString, {valueToConvert}, "intToString", false);
		}
		else
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::uIntToString, {valueToConvert}, "uIntToString", false);
		}
	}
	else if (valueToConvert->getType() == llvmTypes.longintType)
	{
		if (fromType.isSignedType())
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::int64ToString, {valueToConvert}, "int64ToString", false);
		}
		else
		{
			return createIntrinsicCall(context, &LLVMCatIntrinsics::uInt64ToString, {valueToConvert}, "uInt64ToString", false);
		}
	}
	else
	{
		return valueToConvert;
	}
}


llvm::Value* LLVMCodeGeneratorHelper::convertToPointer(llvm::Value* addressValue, const std::string& name, llvm::PointerType* type)
{
	if (type == nullptr)
	{
		type = llvmTypes.pointerType;
	}
	if (addressValue->getType() == type)
	{
		return addressValue;
	}
	else if (addressValue->getType()->isPointerTy())
	{
		return codeGenerator->getBuilder()->CreatePointerCast(addressValue, type, name);
	}
	else
	{
		llvm::Value* intToPtr =  codeGenerator->getBuilder()->CreateIntToPtr(addressValue, type);
		intToPtr->setName(name);
		return intToPtr;
	}
}


llvm::Value* LLVMCodeGeneratorHelper::convertToPointer(llvm::Constant* addressConstant, const std::string& name, llvm::PointerType* type)
{
	if (type == nullptr)
	{
		type = llvmTypes.pointerType;
	}
	return convertToPointer(static_cast<llvm::Value*>(addressConstant), name, type);
}


llvm::Value* LLVMCodeGeneratorHelper::convertToIntPtr(llvm::Value* llvmPointer, const std::string& name)
{
	llvm::Value* ptrToInt = codeGenerator->getBuilder()->CreatePtrToInt(llvmPointer, llvmTypes.uintPtrType);
	ptrToInt->setName(name);
	return ptrToInt;
}


llvm::PointerType* jitcat::LLVM::LLVMCodeGeneratorHelper::getPointerTo(llvm::Type* type) const
{
	return type->getPointerTo();
}


bool LLVMCodeGeneratorHelper::isPointer(llvm::Type* type)
{
	return type == llvmTypes.pointerType;
}


bool LLVMCodeGeneratorHelper::isIntPtr(llvm::Type* type)
{
	return type == llvmTypes.uintPtrType;
}


bool LLVMCodeGeneratorHelper::isInt(llvm::Type* type)
{
	return type == llvmTypes.intType;
}


bool LLVMCodeGeneratorHelper::isPointer(llvm::Value* value)
{
	return value->getType() == llvmTypes.pointerType;
}


bool LLVMCodeGeneratorHelper::isIntPtr(llvm::Value* value)
{
	return value->getType() == llvmTypes.uintPtrType;
}


bool LLVMCodeGeneratorHelper::isInt(llvm::Value* value)
{
	return value->getType() == llvmTypes.intType;
}


llvm::Value* LLVMCodeGeneratorHelper::loadBasicType(llvm::Type* type, llvm::Value* addressValue, const std::string& name)
{
	auto builder = codeGenerator->getBuilder();
	llvm::Value* addressAsPointer = addressValue;
	if (!addressValue->getType()->isPointerTy())
	{
		addressAsPointer = builder->CreateIntToPtr(addressValue, llvm::PointerType::getUnqual(type));
	}
	else if (addressValue->getType() != type->getPointerTo())
	{
		addressAsPointer = builder->CreatePointerCast(addressAsPointer, type->getPointerTo(), "cast");
	}
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
	if (type == nullptr)
	{
		type = llvmTypes.pointerType;
	}
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


llvm::Value* LLVMCodeGeneratorHelper::loadPointerAtAddress(llvm::GlobalVariable* addressValue, const std::string& name, llvm::PointerType* type)
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


llvm::Value* LLVMCodeGeneratorHelper::createOffsetGlobalValue(LLVMCompileTimeContext* context, const std::string& globalName, std::uintptr_t offset)
{
	if (!context->isPrecompilationContext)
	{
		return createIntPtrConstant(context, offset, globalName);
	}
	else
	{
		llvm::GlobalVariable* memberOffsetPtr = std::static_pointer_cast<LLVM::LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalVariable(globalName, context);
		llvm::Value* intPtr = getBuilder()->CreatePointerCast(memberOffsetPtr, llvmTypes.uintPtrType->getPointerTo(), "intptr");
		return loadBasicType(llvmTypes.uintPtrType, intPtr, globalName);
	}
}


llvm::Value* LLVMCodeGeneratorHelper::createTypeInfoGlobalValue(LLVMCompileTimeContext* context, Reflection::TypeInfo* typeInfo)
{
	std::string constantName = Tools::append("_TypeInfo:", typeInfo->getTypeName());
	if (!context->isPrecompilationContext)
	{
		llvm::Constant* typeInfoConstant = createIntPtrConstant(context, reinterpret_cast<uintptr_t>(typeInfo), constantName);
		return convertToPointer(typeInfoConstant, Tools::append(constantName, "_Ptr"));
	}
	else
	{
		llvm::GlobalVariable* memberOffsetPtr = std::static_pointer_cast<LLVM::LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalVariable(constantName, context);
		return loadPointerAtAddress(memberOffsetPtr, constantName);
	}
}


llvm::Constant* LLVMCodeGeneratorHelper::createZeroInitialisedConstant(llvm::Type* type)
{
	if		(type == llvmTypes.boolType)		return createConstant(false);
	else if (type == llvmTypes.floatType)		return createConstant(0.0f);
	else if (type == llvmTypes.doubleType)		return createConstant(0.0);
	else if (type == llvmTypes.intType)			return createConstant(0);
	else if (type == llvmTypes.longintType)		return createConstant((uint64_t)0);
	else if (type == llvmTypes.charType)		return createCharConstant(0);
	else if (type == llvmTypes.voidType)		return (llvm::Constant*)nullptr;
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


llvm::Constant* LLVMCodeGeneratorHelper::createIntPtrConstant(LLVMCompileTimeContext* context, unsigned long long constant, const std::string& name)
{
	assert(!context->isPrecompilationContext && "Pointer constants are not allowed during precompilation.");
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


llvm::Constant* LLVMCodeGeneratorHelper::createZeroTerminatedStringConstant(const std::string& value)
{
    auto charType = llvmTypes.charType;


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


llvm::GlobalVariable* LLVMCodeGeneratorHelper::createGlobalPointerSymbol(const std::string& name)
{
    auto globalDeclaration = (llvm::GlobalVariable*) codeGenerator->getCurrentModule()->getOrInsertGlobal(name, llvmTypes.pointerType);
    globalDeclaration->setInitializer(createNullPtrConstant(llvmTypes.pointerType));
    globalDeclaration->setConstant(false);
    globalDeclaration->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
    globalDeclaration->setUnnamedAddr (llvm::GlobalValue::UnnamedAddr::Global);
	return globalDeclaration;
}


llvm::Value* LLVMCodeGeneratorHelper::createPtrConstant(LLVMCompileTimeContext* context, unsigned long long address, const std::string& name, llvm::PointerType* pointerType)
{
	assert(!context->isPrecompilationContext && "Pointer constants are not allowed during precompilation.");
	llvm::Constant* constant = createIntPtrConstant(context, address, Tools::append(name, "_IntPtr"));
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


llvm::Constant* LLVMCodeGeneratorHelper::globalVariableToConstant(llvm::GlobalVariable* global) const
{
	return static_cast<llvm::Constant*>(global);
}


llvm::Value* LLVMCodeGeneratorHelper::globalVariableToValue(llvm::GlobalVariable* global) const
{
	return static_cast<llvm::Value*>(global);
}


llvm::Value* LLVMCodeGeneratorHelper::createObjectAllocA(LLVMCompileTimeContext* context, const std::string& name, 
														const CatGenericType& objectType, bool generateDestructorCall)
{
	auto builder = codeGenerator->getBuilder();
	llvm::BasicBlock* previousInsertBlock = builder->GetInsertBlock();
	bool currentBlockIsEntryBlock = &context->currentFunction->getEntryBlock() == previousInsertBlock;
	builder->SetInsertPoint(&context->currentFunction->getEntryBlock(), context->currentFunction->getEntryBlock().begin());
	
	llvm::Value* arraySizeVariable = context->helper->createOffsetGlobalValue(context, Tools::append("__sizeOf:", objectType.toString()), objectType.getTypeSize());

	llvm::AllocaInst* objectAllocation = builder->CreateAlloca(llvmTypes.charType, arraySizeVariable, name);

	llvm::Value* objectAllocationAsIntPtr = builder->CreatePointerCast(objectAllocation, llvmTypes.pointerType);

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
		
		assert(objectType.isDestructible());
		context->blockDestructorGenerators.push_back([=]()
			{
				llvm::Value* typeInfoConstantAsIntPtr = createTypeInfoGlobalValue(context, typeInfo);
				return createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementDestructType, {objectAllocationAsIntPtr, typeInfoConstantAsIntPtr}, "_jc_placementDestructType", true);
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


llvm::Value* LLVMCodeGeneratorHelper::generateFunctionCallArgumentNullChecks(const std::vector<llvm::Value*>& arguments, 
																			 const std::vector<int> argumentIndicesToNullCheck,
																			 int argumentsOffset,
																			 LLVMCodeGenerator* generator, LLVMCompileTimeContext* context)
{
	llvm::Value* nullCheckAggregate = nullptr;
	for (auto iter : argumentIndicesToNullCheck)
	{
		int index = iter + argumentsOffset;
		assert(index < (int)arguments.size());
		assert(arguments[index]->getType()->isPointerTy());
		llvm::Value* isNotNull = generator->builder->CreateIsNotNull(arguments[index], arguments[index]->getName());
		if (nullCheckAggregate == nullptr)
		{
			nullCheckAggregate = isNotNull;
		}
		else
		{
			nullCheckAggregate = generator->builder->CreateAnd(nullCheckAggregate, isNotNull);
		}
	}
	return nullCheckAggregate;
}


llvm::Value* LLVMCodeGeneratorHelper::generateStaticFunctionCall(const jitcat::CatGenericType& returnType, const std::vector<llvm::Value*>& argumentList, 
																 const std::vector<llvm::Type*>& argumentTypes, LLVMCompileTimeContext* context, 
																 const std::string& mangledFunctionName, const std::string& shortFunctionName,
																 llvm::Value* returnedObjectAllocation, bool isDirectlyLinked, bool resultIsNonNull)
{
	if (returnType.isReflectableObjectType())
	{
		llvm::FunctionType* functionType = llvm::FunctionType::get(llvmTypes.voidType, argumentTypes, false);
		llvm::CallInst* call = static_cast<llvm::CallInst*>(createCall(context, functionType, argumentList, false, mangledFunctionName, shortFunctionName, isDirectlyLinked));
		call->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
		call->addDereferenceableAttr(1 , returnType.getTypeSize());
		return returnedObjectAllocation;
	}
	else
	{
		llvm::Type* returnLLVMType = toLLVMType(returnType);
		llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
		llvm::CallInst* callInstruction = static_cast<llvm::CallInst*>(createCall(context, functionType, argumentList, false, mangledFunctionName, shortFunctionName, isDirectlyLinked));
		if (resultIsNonNull && callInstruction->getType()->isPointerTy())
		{
			callInstruction->addAttribute(llvm::AttributeList::ReturnIndex, llvm::Attribute::NonNull);
		}
		return callInstruction;
	}
}


llvm::Value* LLVMCodeGeneratorHelper::generateMemberFunctionCall(Reflection::MemberFunctionInfo* memberFunction, const AST::CatTypedExpression* base, 
																const std::vector<const AST::CatTypedExpression*>& arguments, 
																const std::vector<int>& argumentsToCheckForNull,
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
	MemberFunctionCallData callData = memberFunction->getFunctionAddress(FunctionType::Auto);
	assert(callData.functionAddress != 0 || callData.linkDylib || callData.inlineFunctionGenerator != nullptr);
	std::vector<llvm::Value*> argumentList;
	llvm::Value* functionThis = context->helper->convertToPointer(baseObject, memberFunction->getMemberFunctionName() + "_This_Ptr");
	argumentList.push_back(functionThis);
	std::vector<llvm::Type*> argumentTypes;
	argumentTypes.push_back(llvmTypes.pointerType);
	if (callData.callType == MemberFunctionCallType::ThisCallThroughStaticFunction)
	{
		//Add an argument that contains a pointer to a MemberFunctionInfo object.
		llvm::Value* memberFunctionInfoAddressValue = context->helper->generateStaticPointerVariable(callData.functionInfoStructAddress, context, memberFunction->getMangledFunctionInfoName(context->targetConfig->sretBeforeThis, FunctionType::Member));
		llvm::Value* memberFunctionPtrValue = context->helper->convertToPointer(memberFunctionInfoAddressValue, "MemberFunctionInfo_Ptr");
		argumentList.push_back(memberFunctionPtrValue);
		argumentTypes.push_back(llvmTypes.pointerType);
	}
	int offset = (int)argumentList.size();
	generateFunctionCallArgumentEvalatuation(arguments, memberFunction->getArgumentTypes(), argumentList, argumentTypes, codeGenerator, context);
	llvm::Value* argumentNullCheck = generateFunctionCallArgumentNullChecks(argumentList, argumentsToCheckForNull, offset, codeGenerator, context);

	auto notNullCodeGen = [&](LLVMCompileTimeContext* compileContext)
	{
		if (callData.callType != MemberFunctionCallType::InlineFunctionGenerator)
		{
			if (!callData.linkDylib)
			{
				defineWeakSymbol(context, callData.functionAddress, memberFunction->getMangledName(context->targetConfig->sretBeforeThis, FunctionType::Auto), false);
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
					return generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, memberFunction->getMangledName(context->targetConfig->sretBeforeThis, FunctionType::Auto), memberFunction->getMemberFunctionName(), returnedObjectAllocation, false, callData.nonNullResult);
				}
				else
				{
					llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
					llvm::CallInst* call = static_cast<llvm::CallInst*>(compileContext->helper->createCall(context, functionType, argumentList, 
																										   callData.callType == MemberFunctionCallType::ThisCall, 
																										   memberFunction->getMangledName(context->targetConfig->sretBeforeThis, FunctionType::Auto), 
																										   memberFunction->getMemberFunctionName(), false));
					if (codeGenerator->targetConfig->useThisCall && callData.callType == MemberFunctionCallType::ThisCall)
					{
						call->setCallingConv(llvm::CallingConv::X86_ThisCall);
					}
					if (callData.nonNullResult && call->getType()->isPointerTy())
					{
						call->addAttribute(llvm::AttributeList::ReturnIndex, llvm::Attribute::NonNull);
					}
					return static_cast<llvm::Value*>(call);
				}
			}
			else if (callData.callType == MemberFunctionCallType::PseudoMemberCall)
			{
				argumentTypes.insert(argumentTypes.begin(), llvmTypes.pointerType);
				argumentList.insert(argumentList.begin(), returnedObjectAllocation);
				return generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, memberFunction->getMangledName(context->targetConfig->sretBeforeThis, FunctionType::Auto), 
												  memberFunction->getMemberFunctionName(), returnedObjectAllocation, false, callData.nonNullResult);
			}
			else if (returnType.isReflectableObjectType())
			{
				auto sretTypeInsertPoint = argumentTypes.begin();
				if (!codeGenerator->targetConfig->sretBeforeThis && callData.callType == MemberFunctionCallType::ThisCall)
				{
					sretTypeInsertPoint++;
				}
				argumentTypes.insert(sretTypeInsertPoint, llvmTypes.pointerType);

				llvm::FunctionType* functionType = llvm::FunctionType::get(llvmTypes.voidType, argumentTypes, false);
				auto sretInsertPoint = argumentList.begin();
				if (!codeGenerator->targetConfig->sretBeforeThis && callData.callType == MemberFunctionCallType::ThisCall)
				{
					sretInsertPoint++;
				}
				argumentList.insert(sretInsertPoint, returnedObjectAllocation);
				
				llvm::CallInst* call = static_cast<llvm::CallInst*>(compileContext->helper->createCall(context, functionType, argumentList, 
																									   callData.callType == MemberFunctionCallType::ThisCall, 
																									   memberFunction->getMangledName(context->targetConfig->sretBeforeThis, FunctionType::Auto), 
																									   memberFunction->getMemberFunctionName(), false));
				if (codeGenerator->targetConfig->useThisCall && callData.callType == MemberFunctionCallType::ThisCall)
				{
					call->setCallingConv(llvm::CallingConv::X86_ThisCall);
				}
				call->addParamAttr(codeGenerator->targetConfig->sretBeforeThis ? 0 : 1, llvm::Attribute::AttrKind::StructRet);
				call->addDereferenceableAttr(codeGenerator->targetConfig->sretBeforeThis ? 1 : 2, returnType.getTypeSize());
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
	if (argumentNullCheck != nullptr)
	{
		llvm::Value* baseNotNull = getBuilder()->CreateIsNotNull(baseObject, baseObject->getName());
		llvm::Value* bothNotNull = getBuilder()->CreateAnd(baseNotNull, argumentNullCheck);
		return createOptionalNullCheckSelect(bothNotNull, notNullCodeGen, returnType, returnedObjectAllocation, context);
	}
	else
	{
		return createOptionalNullCheckSelect(baseObject, notNullCodeGen, returnType, returnedObjectAllocation, context);
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


void LLVMCodeGeneratorHelper::defineWeakSymbol(LLVMCompileTimeContext* context, intptr_t functionAddress, const std::string& mangledFunctionName, bool isDirectlyLinked)
{
	if (!context->isPrecompilationContext)
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
	else if (!isDirectlyLinked)
	{
		//The function needs to be defined as a global variable function pointer.
		std::static_pointer_cast<LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalFunctionPointer(mangledFunctionName, context);
	}
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
		llvm::Value* copyAllocation = context->helper->createObjectAllocA(context, Tools::append("Argument_", valueName, "_copy"), type, codeGenerator->targetConfig->callerDestroysTemporaryArguments);
		const std::string& typeName = type.getObjectType()->getTypeName();
		llvm::Value* typeInfoConstantAsIntPtr = createTypeInfoGlobalValue(context, type.getObjectType()); 
		assert(type.isCopyConstructible());
		createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementCopyConstructType, {copyAllocation, value, typeInfoConstantAsIntPtr}, "_jc_placementCopyConstructType", true);
		return copyAllocation;
	}
	return value;
}


llvm::Value* LLVMCodeGeneratorHelper::generateIntrinsicCall(jitcat::Reflection::StaticFunctionInfo* functionInfo, std::vector<llvm::Value*>& arguments, 
															LLVMCompileTimeContext* context, bool isDirectlyLinked)
{
	//Define the function in the runtime library.
	defineWeakSymbol(context, functionInfo->getFunctionAddress(), functionInfo->getNormalFunctionName(), isDirectlyLinked);
	
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

	return generateStaticFunctionCall(functionInfo->getReturnType(), arguments, argumentLLVMTypes, context, functionInfo->getNormalFunctionName(), 
									  functionInfo->getNormalFunctionName(), returnValueAllocation, isDirectlyLinked, false);
}
