/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCodeGenerator.h"
#include "jitcat/CatASTNodes.h"
#include "jitcat/Configuration.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMJit.h"
#include "jitcat/LLVMMemoryManager.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/StaticMemberInfo.h"

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>


#include <iostream>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;


LLVMCodeGenerator::LLVMCodeGenerator(const std::string& name):
	currentModule(new llvm::Module("JitCat", LLVMJit::get().getContext())),
	builder(new llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>(LLVMJit::get().getContext())),
	executionSession(new llvm::orc::ExecutionSession()),
	helper(std::make_unique<LLVMCodeGeneratorHelper>(builder.get(), currentModule.get())),
	mangler(new llvm::orc::MangleAndInterner(*executionSession, LLVMJit::get().getDataLayout())),
	objectLinkLayer(new llvm::orc::RTDyldObjectLinkingLayer(*executionSession.get(),
															[]() {	return memoryManager->createExpressionAllocator();})),
	compileLayer(new llvm::orc::IRCompileLayer(*executionSession.get(), *(objectLinkLayer.get()), std::make_unique<llvm::orc::ConcurrentIRCompiler>(LLVMJit::get().getTargetMachineBuilder())))
{
	llvm::orc::SymbolMap intrinsicSymbols;
	runtimeLibraryDyLib = &executionSession->createJITDylib("runtimeLibrary");

	llvm::JITSymbolFlags functionFlags;
	functionFlags |= llvm::JITSymbolFlags::Callable;
	functionFlags |= llvm::JITSymbolFlags::Exported;
	functionFlags |= llvm::JITSymbolFlags::Absolute;

	intrinsicSymbols[executionSession->intern("fmodf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&fmodf), functionFlags);
	intrinsicSymbols[executionSession->intern("_fmod")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&fmodl), functionFlags);
	intrinsicSymbols[executionSession->intern("sinf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&sinf), functionFlags);
	intrinsicSymbols[executionSession->intern("_sin")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&sinl), functionFlags);
	intrinsicSymbols[executionSession->intern("cosf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&cosf), functionFlags);
	intrinsicSymbols[executionSession->intern("_cos")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&cosl), functionFlags);
	intrinsicSymbols[executionSession->intern("log10f")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&log10f), functionFlags);
	intrinsicSymbols[executionSession->intern("_log10")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&log10l), functionFlags);
	intrinsicSymbols[executionSession->intern("logf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&logf), functionFlags);
	intrinsicSymbols[executionSession->intern("_log")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&logl), functionFlags);
	intrinsicSymbols[executionSession->intern("expf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&expf), functionFlags);
	intrinsicSymbols[executionSession->intern("_exp")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&expl), functionFlags);
	intrinsicSymbols[executionSession->intern("powf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&powf), functionFlags);
	intrinsicSymbols[executionSession->intern("_pow")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&powl), functionFlags);
	intrinsicSymbols[executionSession->intern("ceilf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&ceilf), functionFlags);
	intrinsicSymbols[executionSession->intern("_ceil")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&ceill), functionFlags);
	intrinsicSymbols[executionSession->intern("floorf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&floorf), functionFlags);
	intrinsicSymbols[executionSession->intern("_floor")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&floorl), functionFlags);
	

	llvm::cantFail(runtimeLibraryDyLib->define(llvm::orc::absoluteSymbols(intrinsicSymbols)));

	dylib = &executionSession->createJITDylib(Tools::append(name, "_", 0));
	dylib->addToSearchOrder(*runtimeLibraryDyLib);

	if constexpr (Configuration::enableSymbolSearchWorkaround)
	{
		objectLinkLayer->setAutoClaimResponsibilityForObjectSymbols(true);
		objectLinkLayer->setOverrideObjectFlagsWithResponsibilityFlags(true);
	}

	std::string targetTriple = LLVMJit::get().getTargetMachine().getTargetTriple().str();
	currentModule->setTargetTriple(targetTriple);
	currentModule->setDataLayout(LLVMJit::get().getDataLayout());
	helper->setCurrentModule(currentModule.get());
	// Create a new pass manager attached to it.
	passManager = std::make_unique<llvm::legacy::FunctionPassManager>(currentModule.get());

	// Do simple "peephole" optimizations and bit-twiddling optzns.
	passManager->add(llvm::createInstructionCombiningPass());
	// Reassociate expressions.
	passManager->add(llvm::createReassociatePass());
	// Eliminate Common SubExpressions.
	passManager->add(llvm::createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	passManager->add(llvm::createCFGSimplificationPass());

	passManager->doInitialization();

}


LLVMCodeGenerator::~LLVMCodeGenerator()
{
}


llvm::Value* LLVMCodeGenerator::generate(const CatTypedExpression* expression, LLVMCompileTimeContext* context)
{
	initContext(context);
	switch (expression->getNodeType())
	{
		case CatASTNodeType::ArrayIndex:			return generate(static_cast<const CatArrayIndex*>(expression), context);		
		case CatASTNodeType::AssignmentOperator:	return generate(static_cast<const CatAssignmentOperator*>(expression), context);	
		case CatASTNodeType::BuiltInFunctionCall:	return generate(static_cast<const CatBuiltInFunctionCall*>(expression), context);		
		case CatASTNodeType::IndirectionConversion:	return generate(static_cast<const CatIndirectionConversion*>(expression), context);
		case CatASTNodeType::InfixOperator:			return generate(static_cast<const CatInfixOperator*>(expression), context);	
		case CatASTNodeType::Literal:				return generate(static_cast<const CatLiteral*>(expression), context);			
		case CatASTNodeType::MemberAccess:			return generate(static_cast<const CatMemberAccess*>(expression), context);		
		case CatASTNodeType::MemberFunctionCall:	return generate(static_cast<const CatMemberFunctionCall*>(expression), context);
		case CatASTNodeType::PrefixOperator:		return generate(static_cast<const CatPrefixOperator*>(expression), context);	
		case CatASTNodeType::ScopeRoot:				return generate(static_cast<const CatScopeRoot*>(expression), context);		
		case CatASTNodeType::StaticFunctionCall:	return generate(static_cast<const CatStaticFunctionCall*>(expression), context);
		case CatASTNodeType::StaticMemberAccess:	return generate(static_cast<const CatStaticMemberAccess*>(expression), context);
	}
	assert(false);
	return nullptr;
}


llvm::Function* LLVMCodeGenerator::generateExpressionFunction(const CatTypedExpression* expression, LLVMCompileTimeContext* context, const std::string& name)
{
	initContext(context);
	
	//Define the parameters for the function.
	//Each function will at least get a CatRuntimeContext* as a parameter.
	//If the function returns a string by value, the string is returned through a string pointer parameter and the function returns void.
	//The string pointer should point to pre-allocated memory where the returned string will be constructed.
	std::vector<llvm::Type*> parameters;
	llvm::FunctionType* functionType = nullptr;
	CatGenericType expressionType = expression->getType();
	if (expressionType.isStringType())
	{
		parameters.push_back(LLVMTypes::stringPtrType);
		parameters.push_back(LLVMTypes::pointerType);
		functionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);		
	}
	else if (expressionType.isReflectableObjectType())
	{
		parameters.push_back(LLVMTypes::pointerType);
		parameters.push_back(LLVMTypes::pointerType);
		functionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);		
	}
	else
	{
		parameters.push_back(LLVMTypes::pointerType);
		functionType = llvm::FunctionType::get(helper->toLLVMType(expressionType), parameters, false);
	}

	//Create the function signature. No code is yet associated with the function at this time.
	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, name.c_str(), currentModule.get());

	//Attributes and names for the parameters can now be set on the function signature.
	//When returning a string, the StructRet attribute is set to indicate that the parameter is used for returning a structure by value.
	llvm::Argument* firstArg = function->arg_begin();
	if (expressionType.isStringType() || expressionType.isReflectableObjectType())
	{
		firstArg->setName(expressionType.isStringType() ? "stringRet" : Tools::append(expressionType.getObjectTypeName(), "Ret"));
		firstArg++;
		firstArg->setName("RuntimeContext");
		function->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
		function->addParamAttr(0, llvm::Attribute::AttrKind::NoAlias);
	}
	else
	{
		firstArg->setName("RuntimeContext");
	}
	
	//Now, generate code for the function
	context->currentFunction = function;
	
	//Function entry block
	llvm::BasicBlock* block = llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
	builder->SetInsertPoint(&function->getEntryBlock());

	//Generate code for the expression.
	llvm::Value* expressionValue = generate(expression, context);

	//If the expression returns a string or a reflectable object, copy construct it into the StructRet parameter and return void.
	//If it is some other type, just return the value.
	if (expressionType.isStringType())
	{
		helper->createCall(context, &LLVMCatIntrinsics::stringCopyConstruct, {function->arg_begin(), expressionValue}, "stringCopyConstruct");
		helper->generateBlockDestructors(context);
		builder->CreateRetVoid();
	}
	else if (expressionType.isReflectableObjectType())
	{
		llvm::Constant* typeInfoConstant = helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(expressionType.getObjectType()), Tools::append(name, "_typeInfo"));
		llvm::Value* typeInfoConstantAsIntPtr = helper->convertToPointer(typeInfoConstant, Tools::append(name, "_typeInfoPtr"));
		assert(expressionType.isCopyConstructible());
		llvm::Value* castPointer = builder->CreatePointerCast(expressionValue, LLVMTypes::pointerType, Tools::append(name, "_ObjectPointerCast"));
		helper->createOptionalNullCheckSelect(castPointer, 
			[&](LLVMCompileTimeContext* context)
			{
				return helper->createCall(context, &LLVMCatIntrinsics::placementCopyConstructType, {function->arg_begin(), castPointer, typeInfoConstantAsIntPtr}, Tools::append(name, "_copyConstructor"));
			},
			[&](LLVMCompileTimeContext* context)
			{
				return helper->createCall(context, &LLVMCatIntrinsics::placementConstructType, {function->arg_begin(), typeInfoConstantAsIntPtr},  Tools::append(name, "_defaultConstructor"));
			}, context);

		helper->generateBlockDestructors(context);
		builder->CreateRetVoid();
	}
	else
	{
		helper->generateBlockDestructors(context);
		builder->CreateRet(expressionValue);
	}
	context->currentFunction = nullptr;

	//Verify the correctness of the function and execute optimization passes.
	return verifyAndOptimizeFunction(function);
}


llvm::Function* LLVMCodeGenerator::generateExpressionAssignFunction(const CatAssignableExpression* expression, LLVMCompileTimeContext* context, const std::string& name)
{
	initContext(context);
	
	//Define the parameters for the function.
	//Assign functions will always have two parameters: a CatRuntimeContext* and a value that will be assigned to the result of the assignable expression.
	//It will always return void.
	std::vector<llvm::Type*> parameters = {LLVMTypes::pointerType, helper->toLLVMType(expression->getType())};
	llvm::FunctionType* functionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);		

	//Create the function signature. No code is yet associated with the function at this time.
	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, name.c_str(), currentModule.get());

	//Name the parameters
	llvm::Argument* argIter = function->arg_begin();
	argIter->setName("RuntimeContext");
	++argIter;
	argIter->setName("ValueToAssign");

	//Now, generate code for the function
	context->currentFunction = function;

	//Function entry block
	llvm::BasicBlock* block = llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
	builder->SetInsertPoint(&function->getEntryBlock());

	//Generate the assignment expression, passing in the second function parameter.
	generateAssign(expression, argIter, context);

	//Destruct any heap allocations done by the function
	helper->generateBlockDestructors(context);
	
	//Create the return statement, returning void.
	builder->CreateRetVoid();

	//Verify the correctness of the function and execute optimization passes.
	return verifyAndOptimizeFunction(function);
}


intptr_t LLVMCodeGenerator::generateAndGetFunctionAddress(const CatTypedExpression* expression, LLVMCompileTimeContext* context)
{
	initContext(context);
	createNewModule(context);
	std::string functionName = getNextFunctionName(context);
	llvm::Function* function = generateExpressionFunction(expression, context, functionName);
	assert(function != nullptr);
	llvm::cantFail(compileLayer->add(*dylib, llvm::orc::ThreadSafeModule(std::move(currentModule), LLVMJit::get().getThreadSafeContext())));
	return (intptr_t)getSymbolAddress(functionName.c_str(), *dylib);
}


intptr_t LLVMCodeGenerator::generateAndGetAssignFunctionAddress(const CatAssignableExpression* expression, LLVMCompileTimeContext* context)
{
	initContext(context);
	createNewModule(context);
	std::string functionName = getNextFunctionName(context);
	llvm::Function* function = generateExpressionAssignFunction(expression, context, functionName);
	assert(function != nullptr);
	llvm::cantFail(compileLayer->add(*dylib, llvm::orc::ThreadSafeModule(std::move(currentModule), LLVMJit::get().getThreadSafeContext())));
	return (intptr_t)getSymbolAddress(functionName.c_str(), *dylib);
}


llvm::Value* LLVMCodeGenerator::generate(const CatBuiltInFunctionCall* functionCall, LLVMCompileTimeContext* context)
{
	CatArgumentList* arguments = functionCall->getArgumentList();
	switch (functionCall->getFunctionType())
	{
		case CatBuiltInFunctionType::ToVoid:	generate(arguments->getArgument(0), context); return nullptr;
		case CatBuiltInFunctionType::ToInt:		return helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::intType, context);
		case CatBuiltInFunctionType::ToFloat:	return helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context);
		case CatBuiltInFunctionType::ToBool:	return helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::boolType, context);
		case CatBuiltInFunctionType::Select:
		{
			llvm::Value* conditionValue = helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::boolType, context);
			llvm::Value* trueValue = generate(arguments->getArgument(1), context);
			llvm::Value* falseValue = helper->convertType(generate(arguments->getArgument(2), context), trueValue->getType(), context);
			return builder->CreateSelect(conditionValue, trueValue, falseValue);
		}
		case CatBuiltInFunctionType::Abs:
		{
			if (arguments->getArgumentType(0).isFloatType())
			{
				return helper->callIntrinsic(llvm::Intrinsic::fabs, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
			}
			else if (arguments->getArgumentType(0) == CatGenericType::intType)
			{
				llvm::Value* intValue = generate(arguments->getArgument(0), context);
				llvm::Value* isGreaterOrEqualToZero = builder->CreateICmpSGT(intValue, helper->createConstant(-1));
				llvm::Value* negated = builder->CreateSub(helper->createConstant(0), intValue);
				return builder->CreateSelect(isGreaterOrEqualToZero, intValue, negated);
			}
			return nullptr;
		}
		case CatBuiltInFunctionType::Log10:	return helper->callIntrinsic(llvm::Intrinsic::log10, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
		case CatBuiltInFunctionType::Sqrt:	return helper->callIntrinsic(llvm::Intrinsic::sqrt, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
		case CatBuiltInFunctionType::Ln:	return helper->callIntrinsic(llvm::Intrinsic::log, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
		case CatBuiltInFunctionType::Exp:	return helper->callIntrinsic(llvm::Intrinsic::exp, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
		case CatBuiltInFunctionType::Pow:
		{
			return helper->callIntrinsic(llvm::Intrinsic::pow, CatGenericType::floatType, generate(arguments->getArgument(0), context), generate(arguments->getArgument(1), context), context);
		}
		case CatBuiltInFunctionType::Sin:		return helper->callIntrinsic(llvm::Intrinsic::sin, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
		case CatBuiltInFunctionType::Cos:		return helper->callIntrinsic(llvm::Intrinsic::cos, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
		case CatBuiltInFunctionType::Ceil:		return helper->callIntrinsic(llvm::Intrinsic::ceil, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
		case CatBuiltInFunctionType::Floor:		return helper->callIntrinsic(llvm::Intrinsic::floor, CatGenericType::floatType, generate(arguments->getArgument(0), context), context);
		case CatBuiltInFunctionType::Tan:		return builder->CreateFDiv(helper->callIntrinsic(llvm::Intrinsic::sin, CatGenericType::floatType, generate(arguments->getArgument(0), context), context), helper->callIntrinsic(llvm::Intrinsic::cos, CatGenericType::floatType, generate(arguments->getArgument(0), context), context));

		case CatBuiltInFunctionType::Asin:
			return helper->createCall(context, &std::asinf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "asin");
		case CatBuiltInFunctionType::Acos:
			return helper->createCall(context, &std::acosf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "acos");
		case CatBuiltInFunctionType::Atan:
			return helper->createCall(context, &std::atanf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "atan");

		case CatBuiltInFunctionType::Sinh:
			return helper->createCall(context, &std::sinhf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "sinh");
		case CatBuiltInFunctionType::Cosh:
			return helper->createCall(context, &std::coshf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "cosh");
		case CatBuiltInFunctionType::Tanh:
			return helper->createCall(context, &std::tanhf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "tanh");

		case CatBuiltInFunctionType::Asinh:
			return helper->createCall(context, &std::asinhf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "asinh");
		case CatBuiltInFunctionType::Acosh:
			return helper->createCall(context, &std::acoshf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "acosh");
		case CatBuiltInFunctionType::Atanh:
			return helper->createCall(context, &std::atanhf, { helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context) }, "atanh");

		case CatBuiltInFunctionType::Atan2:
		{
			llvm::Value* y = helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context);
			llvm::Value* x = helper->convertType(generate(arguments->getArgument(1), context), LLVMTypes::floatType, context);
			return helper->createCall(context, &std::atan2f, { y, x }, "atan2");
		}
		case CatBuiltInFunctionType::Hypot:
		{
			llvm::Value* x = helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::floatType, context);
			llvm::Value* y = helper->convertType(generate(arguments->getArgument(1), context), LLVMTypes::floatType, context);
			return helper->createCall(context, &std::hypotf, { x, y }, "hypot");
		}

		case CatBuiltInFunctionType::Cap:
		{
			llvm::Value* value = generate(arguments->getArgument(0), context);
			llvm::Value* minValue = generate(arguments->getArgument(1), context);
			llvm::Value* maxValue = generate(arguments->getArgument(2), context);
			if (arguments->getArgumentType(0).isFloatType())
			{
				llvm::Value* convertedMin = helper->convertType(minValue, LLVMTypes::floatType, context);
				llvm::Value* convertedMax = helper->convertType(maxValue, LLVMTypes::floatType, context);
				llvm::Value* maxSmallerThanMin = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OLT, convertedMax, convertedMin);
				llvm::Value* finalMin = builder->CreateSelect(maxSmallerThanMin, convertedMax, convertedMin);
				llvm::Value* finalMax = builder->CreateSelect(maxSmallerThanMin, convertedMin, convertedMax);
				llvm::Value* smallerThanMin = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OLT, value, finalMin);
				llvm::Value* greaterThanMax = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OGT, value, finalMax);
				llvm::Value* cappedToMin = builder->CreateSelect(smallerThanMin, finalMin, value);
				llvm::Value* cappedToMax = builder->CreateSelect(greaterThanMax, finalMax, cappedToMin);
				return cappedToMax;
			}
			else if (arguments->getArgumentType(0).isIntType())
			{
				llvm::Value* convertedMin = helper->convertType(minValue, LLVMTypes::intType, context);
				llvm::Value* convertedMax = helper->convertType(maxValue, LLVMTypes::intType, context);
				llvm::Value* maxSmallerThanMin = builder->CreateICmp(llvm::CmpInst::Predicate::ICMP_SLT, convertedMax, convertedMin);
				llvm::Value* finalMin = builder->CreateSelect(maxSmallerThanMin, convertedMax, convertedMin);
				llvm::Value* finalMax = builder->CreateSelect(maxSmallerThanMin, convertedMin, convertedMax);
				llvm::Value* smallerThanMin = builder->CreateICmp(llvm::CmpInst::Predicate::ICMP_SLT, value, finalMin);
				llvm::Value* greaterThanMax = builder->CreateICmp(llvm::CmpInst::Predicate::ICMP_SGT, value, finalMax);
				llvm::Value* cappedToMin = builder->CreateSelect(smallerThanMin, finalMin, value);
				llvm::Value* cappedToMax = builder->CreateSelect(greaterThanMax, finalMax, cappedToMin);
				return cappedToMax;
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}
		case CatBuiltInFunctionType::Min:
		{
			llvm::Value* value1 = generate(arguments->getArgument(0), context);
			llvm::Value* value2 = generate(arguments->getArgument(1), context);
			if (arguments->getArgumentType(0).isFloatType())
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, LLVMTypes::floatType, context);
				llvm::Value* lessThan = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OLT, value1, convertedValue2);
				return builder->CreateSelect(lessThan, value1, convertedValue2);
			}
			else if (arguments->getArgumentType(0).isIntType())
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, LLVMTypes::intType, context);
				llvm::Value* lessThan = builder->CreateICmp(llvm::CmpInst::Predicate::ICMP_SLT, value1, convertedValue2);
				return builder->CreateSelect(lessThan, value1, convertedValue2);
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}
		case CatBuiltInFunctionType::Max:
		{
			llvm::Value* value1 = generate(arguments->getArgument(0), context);
			llvm::Value* value2 = generate(arguments->getArgument(1), context);
			if (arguments->getArgumentType(0).isFloatType())
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, LLVMTypes::floatType, context);
				llvm::Value* greaterThan = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OGT, value1, convertedValue2);
				return builder->CreateSelect(greaterThan, value1, convertedValue2);
			}
			else if (arguments->getArgumentType(0).isIntType())
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, LLVMTypes::intType, context);
				llvm::Value* greaterThan = builder->CreateICmp(llvm::CmpInst::Predicate::ICMP_SGT, value1, convertedValue2);
				return builder->CreateSelect(greaterThan, value1, convertedValue2);
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}
		case CatBuiltInFunctionType::FindInString:
		{
			llvm::Value* text = helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::stringPtrType, context);
			llvm::Value* textToFind = helper->convertType(generate(arguments->getArgument(1), context), LLVMTypes::stringPtrType, context);
			return helper->createCall(context, &LLVMCatIntrinsics::findInString, {text, textToFind}, "findInString");
		}
		case CatBuiltInFunctionType::ReplaceInString:
		{
			llvm::Value* sourceString = helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::stringPtrType, context);
			llvm::Value* stringToFind = helper->convertType(generate(arguments->getArgument(1), context), LLVMTypes::stringPtrType, context);
			llvm::Value* replacementString = helper->convertType(generate(arguments->getArgument(2), context), LLVMTypes::stringPtrType, context);
			return helper->createCall(context, &LLVMCatIntrinsics::replaceInString, {sourceString, stringToFind, replacementString}, "replaceInString");
		}
		case CatBuiltInFunctionType::StringLength:
		{
			llvm::Value* string = helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::stringPtrType, context);
			return helper->createCall(context, &LLVMCatIntrinsics::stringLength, {string}, "stringLength");
		}
		case CatBuiltInFunctionType::SubString:
		{
			llvm::Value* sourceString = helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::stringPtrType, context);
			llvm::Value* offset = helper->convertType(generate(arguments->getArgument(1), context), LLVMTypes::intType, context);
			llvm::Value* length = helper->convertType(generate(arguments->getArgument(2), context), LLVMTypes::intType, context);
			return helper->createCall(context, &LLVMCatIntrinsics::subString, {sourceString, offset, length}, "subString");
		}
		case CatBuiltInFunctionType::ToString:
		{
			return helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::stringPtrType, context);
		}
		case CatBuiltInFunctionType::ToPrettyString:
		{
			if (arguments->getArgumentType(0).isIntType())
			{
				return helper->createCall(context, &LLVMCatIntrinsics::intToPrettyString, {generate(arguments->getArgument(0), context)}, "intToPrettyString");
			}
			else
			{
				return helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::stringPtrType, context);
			}
		}
		case CatBuiltInFunctionType::ToFixedLengthString:
		{
			llvm::Value* number = helper->convertType(generate(arguments->getArgument(0), context), LLVMTypes::intType, context);
			llvm::Value* length = helper->convertType(generate(arguments->getArgument(1), context), LLVMTypes::intType, context);
			return helper->createCall(context, &LLVMCatIntrinsics::intToFixedLengthString, {number, length}, "intToFixedLengthString");
		}
		case CatBuiltInFunctionType::Random:
		{
			return helper->createCall(context, &LLVMCatIntrinsics::getRandomFloat, {}, "getRandomFloat");
		}
		case CatBuiltInFunctionType::RandomRange:
		{
			llvm::Value* left = generate(arguments->getArgument(0), context);
			llvm::Value* right = generate(arguments->getArgument(1), context);
			if (arguments->getArgumentType(0).isBoolType()
				&& arguments->getArgumentType(1).isBoolType())
			{
				return helper->createCall(context, &LLVMCatIntrinsics::getRandomBoolean, {left, right}, "getRandomBoolean");
			}
			else if (arguments->getArgumentType(0).isIntType()
					 && arguments->getArgumentType(1).isIntType())
			{
				return helper->createCall(context, &LLVMCatIntrinsics::getRandomInt, {left, right}, "getRandomInt");
			}
			else				
			{
				llvm::Value* leftFloat = helper->convertType(left, LLVMTypes::floatType, context);
				llvm::Value* rightFloat = helper->convertType(right, LLVMTypes::floatType, context);
				return helper->createCall(context, &LLVMCatIntrinsics::getRandomFloatRange, {leftFloat, rightFloat}, "getRandomFloat");
			}
		}
		case CatBuiltInFunctionType::Round:
		{
			llvm::Value* left = generate(arguments->getArgument(0), context);
			llvm::Value* right = generate(arguments->getArgument(1), context);
			llvm::Value* leftFloat = helper->convertType(left, LLVMTypes::floatType, context);
			llvm::Value* rightInt = helper->convertType(right, LLVMTypes::intType, context);
			return helper->createCall(context, &LLVMCatIntrinsics::roundFloat, {leftFloat, rightInt}, "roundFloat");
		}
		case CatBuiltInFunctionType::StringRound:
		{
			llvm::Value* left = generate(arguments->getArgument(0), context);
			llvm::Value* right = generate(arguments->getArgument(1), context);
			llvm::Value* leftFloat = helper->convertType(left, LLVMTypes::floatType, context);
			llvm::Value* rightInt = helper->convertType(right, LLVMTypes::intType, context);
			return helper->createCall(context, &LLVMCatIntrinsics::roundFloatToString, {leftFloat, rightInt}, "roundFloatToString");
		}
		default:
		{
			assert(false);
			return nullptr;
		}
	}
}


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generate(const CatIndirectionConversion* indirectionConversion, LLVMCompileTimeContext* context)
{
	const CatTypedExpression* expressionToConvert = indirectionConversion->getExpressionToConvert();
	llvm::Value* generatedExpression = generate(expressionToConvert, context);
	switch (indirectionConversion->getIndirectionConversionMode())
	{
		case IndirectionConversionMode::DereferencePointer:
		{
			assert(generatedExpression->getType()->isPointerTy());
			assert(expressionToConvert->getType().isPointerType());
			//Value can only be dereferenced if the pointer points to a basic type
			if (expressionToConvert->getType().getPointeeType()->isBasicType())
			{
				//Dereference the pointer
				return builder->CreateLoad(generatedExpression, generatedExpression->getType()->getPointerElementType(), "Dereference");				
			}
			else
			{
				return generatedExpression;
			}
		}
		case IndirectionConversionMode::DereferencePointerToPointer:
		{
			//Check that we are dealing with a pointer to a pointer
			assert(generatedExpression->getType()->isPointerTy());
			assert(generatedExpression->getType()->getPointerElementType()->isPointerTy());
			//Dereference the pointer
			return builder->CreateLoad(generatedExpression, generatedExpression->getType()->getPointerElementType(), "Dereference");				
		}
		case IndirectionConversionMode::DereferencePointerToPointerTwice:
		{
			//Check that we are dealing with a pointer to a pointer
			assert(generatedExpression->getType()->isPointerTy());
			assert(generatedExpression->getType()->getPointerElementType()->isPointerTy());
			//Dereference the pointer
			llvm::Value* value = builder->CreateLoad(generatedExpression, generatedExpression->getType()->getPointerElementType(), "Dereference");				
			if (expressionToConvert->getType().getPointeeType()->getPointeeType()->isBasicType())
			{
				value = builder->CreateLoad(value, value->getType()->getPointerElementType(), "Dereference");				
			}
			return value;
		}
		case IndirectionConversionMode::AddressOfPointer:
		{
			//Check that we are dealing with a pointer
			assert(generatedExpression->getType()->isPointerTy());
			llvm::Value* valuePtr = builder->CreateAlloca(generatedExpression->getType(), nullptr, "ValueMemory");
			builder->CreateStore(generatedExpression, valuePtr);
			return  valuePtr;
		}
		case IndirectionConversionMode::AddressOfValue:
		{
			if (expressionToConvert->getType().isBasicType())
			{
				llvm::Value* valuePtr = builder->CreateAlloca(generatedExpression->getType(), nullptr, "ValueMemory");
				builder->CreateStore(generatedExpression, valuePtr);
				return valuePtr;
			}
			else
			{
				return generatedExpression;
			}
		}
		default: return generatedExpression;
		
	}
}


llvm::Value* LLVMCodeGenerator::generate(const CatInfixOperator* infixOperator, LLVMCompileTimeContext* context)
{
	llvm::Value* left = generate(infixOperator->getLeft(), context);
	llvm::Value* right = generate(infixOperator->getRight(), context);
	CatInfixOperatorType oper = infixOperator->getOperatorType();

	assert(left != nullptr && right != nullptr);

	if (oper == CatInfixOperatorType::LogicalOr)
	{
		return builder->CreateOr(helper->convertType(left, LLVMTypes::boolType, context), helper->convertType(right, LLVMTypes::boolType, context), "or");
	}
	else if (oper == CatInfixOperatorType::LogicalAnd)
	{
		return builder->CreateAnd(helper->convertType(left, LLVMTypes::boolType, context), helper->convertType(right, LLVMTypes::boolType, context), "and");
	}
	else if (left->getType() != right->getType())
	{
		//Operators must always operate on identical types.
		//If types are different, we must first convert them.
		if (left->getType() == LLVMTypes::stringPtrType
			|| right->getType() == LLVMTypes::stringPtrType)
		{
			left = helper->convertType(left, LLVMTypes::stringPtrType, context);
			right = helper->convertType(right, LLVMTypes::stringPtrType, context);
		}
		else if (left->getType() == LLVMTypes::floatType
			|| right->getType() == LLVMTypes::floatType)
		{
			left = helper->convertType(left, LLVMTypes::floatType, context);
			right = helper->convertType(right, LLVMTypes::floatType, context);
		}
		else
		{
			//left and right are ints or booleans, booleans will be promoted to ints
			left = helper->convertType(left, LLVMTypes::intType, context);
			right = helper->convertType(right, LLVMTypes::intType, context);
		}
	}
	if (left->getType() == LLVMTypes::floatType)
	{
		switch (oper)
		{
			case CatInfixOperatorType::Plus:				return builder->CreateFAdd(left, right, "added");				
			case CatInfixOperatorType::Minus:				return builder->CreateFSub(left, right, "subtracted");		
			case CatInfixOperatorType::Multiply:			return builder->CreateFMul(left, right, "multiplied");		
			case CatInfixOperatorType::Divide:
			{
				if constexpr (Configuration::divisionByZeroYieldsZero)
				{
					return builder->CreateSelect(builder->CreateFCmpUEQ(right, helper->createConstant(0.0f)), helper->createConstant(0.0f), builder->CreateFDiv(left, right, "divided"));			
				}
				else
				{
					return builder->CreateFDiv(left, right, "divided");
				}
			}
			case CatInfixOperatorType::Modulo:
			{
				if constexpr (Configuration::divisionByZeroYieldsZero)
				{
					return builder->CreateSelect(builder->CreateFCmpUEQ(right, helper->createConstant(0.0f)), helper->createConstant(0.0f), builder->CreateFRem(left, right, "divided"));
				}
				else
				{
					return builder->CreateFRem(left, right, "divided");
				}
			}
			case CatInfixOperatorType::Greater:				return builder->CreateFCmpUGT(left, right, "greater");		
			case CatInfixOperatorType::Smaller:				return builder->CreateFCmpULT(left, right, "smaller");		
			case CatInfixOperatorType::GreaterOrEqual:		return builder->CreateFCmpUGE(left, right, "greaterOrEqual");	
			case CatInfixOperatorType::SmallerOrEqual:		return builder->CreateFCmpULE(left, right, "lessOrEqual");	
			case CatInfixOperatorType::Equals:				return builder->CreateFCmpUEQ(left, right, "equal");			
			case CatInfixOperatorType::NotEquals:			return builder->CreateFCmpUNE(left, right, "notEqual");		
		}
	}
	else if (left->getType() == LLVMTypes::intType)
	{
		switch (oper)
		{
			case CatInfixOperatorType::Plus:				return builder->CreateAdd(left, right, "added");				
			case CatInfixOperatorType::Minus:				return builder->CreateSub(left, right, "subtracted");			
			case CatInfixOperatorType::Multiply:			return builder->CreateMul(left, right, "multiplied");			
			case CatInfixOperatorType::Divide:				
			{
				if constexpr (Configuration::divisionByZeroYieldsZero)
				{
					return builder->CreateSelect(builder->CreateICmpEQ(right, helper->createConstant(0)), helper->createConstant(0), builder->CreateSDiv(left, right, "divided"));					 
				}
				else
				{
					return builder->CreateSDiv(left, right, "divided");
				}
			}
			case CatInfixOperatorType::Modulo:				
			{
				if constexpr (Configuration::divisionByZeroYieldsZero)
				{
					return builder->CreateSelect(builder->CreateICmpEQ(right, helper->createConstant(0)), helper->createConstant(0), builder->CreateSRem(left, right, "modulo"));			
				}
				else
				{
					return builder->CreateSRem(left, right, "modulo");
				}
			}
			case CatInfixOperatorType::Greater:				return builder->CreateICmpSGT(left, right, "greater");		
			case CatInfixOperatorType::Smaller:				return builder->CreateICmpSLT(left, right, "smaller");		
			case CatInfixOperatorType::GreaterOrEqual:		return builder->CreateICmpSGE(left, right, "greaterOrEqual");	
			case CatInfixOperatorType::SmallerOrEqual:		return builder->CreateICmpSLE(left, right, "smallerOrEqual");	
			case CatInfixOperatorType::Equals:				return builder->CreateICmpEQ(left, right, "equal");
			case CatInfixOperatorType::NotEquals:			return builder->CreateICmpNE(left, right, "notEqual");		
		}
	}
	else if (left->getType() == LLVMTypes::boolType)
	{
		switch (oper)
		{
			case CatInfixOperatorType::Equals:				return builder->CreateICmpEQ(left, right, "equal");
			case CatInfixOperatorType::NotEquals:			return builder->CreateICmpNE(left, right, "notEqual");		
		}
	}
	else if (left->getType() == LLVMTypes::stringPtrType)
	{
		llvm::Value* stringNullCheck = nullptr;
		if (context->options.enableDereferenceNullChecks)
		{
			llvm::Value* leftNotNull = builder->CreateIsNotNull(left, "leftStringNotNull");
			llvm::Value* rightNotNull = builder->CreateIsNotNull(right, "rightStringNotNull");
			stringNullCheck = builder->CreateAnd(leftNotNull, rightNotNull, "neitherStringNull");
		}
		switch (oper)
		{
			case CatInfixOperatorType::Plus:
			{
				return helper->createOptionalNullCheckSelect(stringNullCheck, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createCall(compileContext, &LLVMCatIntrinsics::stringAppend, {left, right}, "stringAppend");}, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createEmptyStringPtrConstant();}, context);
			}
			case CatInfixOperatorType::Equals:
			{
				return helper->createOptionalNullCheckSelect(stringNullCheck, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createCall(compileContext, &LLVMCatIntrinsics::stringEquals, {left, right}, "stringEquals");}, LLVMTypes::boolType, context);  
			}
			case CatInfixOperatorType::NotEquals:		
			{
				return helper->createOptionalNullCheckSelect(stringNullCheck, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createCall(compileContext, &LLVMCatIntrinsics::stringNotEquals, {left, right}, "stringNotEquals");}, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createConstant(true);}, context);
			}
		}
	}
	assert(false);
	return LLVMJit::logError("ERROR: Invalid operation.");
}


llvm::Value* LLVMCodeGenerator::generate(const CatAssignmentOperator* assignmentOperator, LLVMCompileTimeContext* context)
{
	assert(assignmentOperator->getLhs()->isAssignable());
	return generateAssign(static_cast<CatAssignableExpression*>(assignmentOperator->getLhs()), generate(assignmentOperator->getRhs(), context), context);
}


llvm::Value* LLVMCodeGenerator::generate(const CatLiteral* literal, LLVMCompileTimeContext* context)
{
	CatGenericType literalType = literal->getType();
	if		(literalType.isIntType())		return helper->createConstant(std::any_cast<int>(literal->getValue()));
	else if (literalType.isFloatType())		return helper->createConstant(std::any_cast<float>(literal->getValue()));
	else if (literalType.isBoolType())		return helper->createConstant(std::any_cast<bool>(literal->getValue()));
	else if (literalType.isStringType())
	{
		const std::string& stringReference = std::any_cast<const std::string&>(literal->getValue());
		llvm::Value* stringObjectAddress = helper->createIntPtrConstant(reinterpret_cast<std::uintptr_t>(&stringReference), "stringLiteralAddress");
		return builder->CreateIntToPtr(stringObjectAddress, LLVMTypes::stringPtrType);			
	}
	else if (literalType.isPointerToReflectableObjectType())
	{
		uintptr_t pointerConstant = literalType.getRawPointer(literal->getValue());
		llvm::Value* reflectableAddress = helper->createIntPtrConstant(pointerConstant, "literalObjectAddress");
		return builder->CreateIntToPtr(reflectableAddress, LLVMTypes::pointerType);
	}
	else if (literalType.isEnumType())
	{
		//This value contains a C++ enum type.
		std::any value = literal->getValue();
		std::any underlyingValue = literal->getType().toUnderlyingType(value);
		CatLiteral underlyingLiteral(underlyingValue, literal->getType().getUnderlyingEnumType(), literal->getLexeme());
		return generate(&underlyingLiteral, context);
	}
	else
	{
		assert(false); return LLVMJit::logError("ERROR: Not a basic type."); 
	}
}


llvm::Value* LLVMCodeGenerator::generate(const CatMemberAccess* memberAccess, LLVMCompileTimeContext* context)
{
	llvm::Value* base = generate(memberAccess->getBase(), context);
	return memberAccess->getMemberInfo()->generateDereferenceCode(base, context);
}


llvm::Value* LLVMCodeGenerator::generate(const CatMemberFunctionCall* memberFunctionCall, LLVMCompileTimeContext* context)
{
	const CatArgumentList* arguments = memberFunctionCall->getArguments();
	std::vector<const CatTypedExpression*> expressionArguments;
	for (std::size_t i = 0; i < arguments->getNumArguments(); i++)
	{
		expressionArguments.push_back(arguments->getArgument(i));
	}
	return generateMemberFunctionCall(memberFunctionCall->getMemberFunctionInfo(), memberFunctionCall->getBase(), expressionArguments, context);


}


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generate(const AST::CatStaticFunctionCall* staticFunctionCall, LLVMCompileTimeContext* context)
{
	const CatArgumentList* arguments = staticFunctionCall-> getArguments();
	std::vector<const CatTypedExpression*> expressionArguments;
	for (std::size_t i = 0; i < arguments->getNumArguments(); i++)
	{
		expressionArguments.push_back(arguments->getArgument(i));
	}
	std::vector<llvm::Value*> argumentList;
	std::vector<llvm::Type*> argumentTypes;
	const CatGenericType& returnType = staticFunctionCall->getType();
	llvm::Type* returnLLVMType = context->helper->toLLVMType(returnType);

	llvm::Value* returnAllocation = helper->generateFunctionCallReturnValueAllocation(returnType, staticFunctionCall->getFunctionName(), context);
	if (returnAllocation != nullptr)
	{
		argumentList.push_back(returnAllocation);
		argumentTypes.push_back(returnAllocation->getType());
	}
	helper->generateFunctionCallArgumentEvalatuation(expressionArguments, staticFunctionCall->getExpectedParameterTypes(), argumentList, argumentTypes, this, context);

	if (returnType.isStringType() || returnType.isReflectableObjectType())
	{
		llvm::FunctionType* functionType = llvm::FunctionType::get(LLVMTypes::voidType, argumentTypes, false);
		llvm::CallInst* call = static_cast<llvm::CallInst*>(context->helper->createCall(functionType, staticFunctionCall->getFunctionAddress(), argumentList, staticFunctionCall->getFunctionName()));
		call->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
		call->addDereferenceableAttr(1 , returnType.getTypeSize());
		return returnAllocation;
	}
	else
	{
		llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
		return helper->createCall(functionType, staticFunctionCall->getFunctionAddress(), argumentList, staticFunctionCall->getFunctionName());
	}
}


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generate(const AST::CatStaticMemberAccess* staticIdentifier, LLVMCompileTimeContext* context)
{
	const StaticMemberInfo* staticMemberInfo = staticIdentifier->getStaticMemberInfo();

	llvm::Value* result = staticMemberInfo->generateDereferenceCode(context);
	if (result != nullptr)
	{
		return result;
	}
	else
	{
		assert(false);
		return LLVMJit::logError("ERROR: Not yet supported.");
	}
}


llvm::Value* LLVMCodeGenerator::generate(const CatPrefixOperator* prefixOperator, LLVMCompileTimeContext* context)
{
	llvm::Value* right = generate(prefixOperator->getRHS(), context);
	assert(right != nullptr);
	llvm::Type* rightType = right->getType();
	if (!(rightType == LLVMTypes::intType || rightType == LLVMTypes::boolType || rightType == LLVMTypes::floatType))
	{
		assert(false);
		return LLVMJit::logError("ERROR: Type not yet supported for prefix operators.");
	}
	if (prefixOperator->getOperator() == CatPrefixOperator::Operator::Not)
	{
		return builder->CreateNot(helper->convertType(right, LLVMTypes::boolType, context), "not");
	}
	else if (prefixOperator->getOperator() == CatPrefixOperator::Operator::Minus)
	{
		if (rightType == LLVMTypes::floatType)
		{
			return builder->CreateFNeg(right, "negative");
		}
		else
		{
			return builder->CreateNeg(helper->convertType(right, LLVMTypes::intType, context), "negative");
		}
	}
	else
	{
		assert(false);
		return LLVMJit::logError("ERROR: Operator not implemented.");
	}
}


llvm::Value* LLVMCodeGenerator::generate(const CatArrayIndex* arrayIndex, LLVMCompileTimeContext* context)
{
	CatTypedExpression* base = arrayIndex->getBase();
	CatTypedExpression* index = arrayIndex->getIndex();
	if (arrayIndex->isReflectedArray())
	{
		return generateMemberFunctionCall(arrayIndex->getArrayIndexOperatorFunction(), base, {index}, context);
	}
	else if (base->getNodeType() == CatASTNodeType::StaticMemberAccess)
	{
		CatStaticMemberAccess* staticMemberAccess = static_cast<CatStaticMemberAccess*>(base);
		llvm::Value* containerAddress = generate(staticMemberAccess, context);
		llvm::Value* containerIndex = generate(index, context);
		return staticMemberAccess->getStaticMemberInfo()->generateArrayIndexCode(containerAddress, containerIndex, context);
	}
	else if (base->getNodeType() == CatASTNodeType::MemberAccess)
	{
		const TypeMemberInfo* memberInfo = static_cast<CatMemberAccess*>(base)->getMemberInfo();
		assert(memberInfo != nullptr);

		llvm::Value* containerAddress = generate(base, context);
		llvm::Value* containerIndex = generate(index, context);
		return memberInfo->generateArrayIndexCode(containerAddress, containerIndex, context);
	}
	else
	{
		assert(false);
		return nullptr;
	}
}


llvm::Value* LLVMCodeGenerator::generate(const CatScopeRoot* scopeRoot, LLVMCompileTimeContext* context)
{
	return getBaseAddress(scopeRoot->getScopeId(), context);
}


llvm::Value* LLVMCodeGenerator::generateAssign(const CatAssignableExpression* expression, llvm::Value* rValue, LLVMCompileTimeContext* context)
{
	context->helper = helper.get();
	context->currentDyLib = dylib;
	switch (expression->getNodeType())
	{
		case CatASTNodeType::StaticIdentifier:	return generateAssign(static_cast<const CatStaticIdentifier*>(expression), rValue, context);
		case CatASTNodeType::MemberAccess:	return generateAssign(static_cast<const CatMemberAccess*>(expression), rValue, context);
	}
	assert(false);
	return nullptr;
}


llvm::Value* LLVMCodeGenerator::generateAssign(const CatMemberAccess* memberAccess, llvm::Value* rValue, LLVMCompileTimeContext* context)
{
	llvm::Value* base = generate(memberAccess->getBase(), context);
	return memberAccess->getMemberInfo()->generateAssignCode(base, rValue, context);
}


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generateAssign(const AST::CatStaticMemberAccess* memberAccess, llvm::Value* rValue, LLVMCompileTimeContext* context)
{
	const StaticMemberInfo* staticMemberInfo = memberAccess->getStaticMemberInfo();

	llvm::Value* result = staticMemberInfo->generateAssignCode(rValue, context);
	if (result != nullptr)
	{
		return result;
	}
	else
	{
		assert(false);
		return LLVMJit::logError("ERROR: Not supported.");
	}
}


llvm::Value* LLVMCodeGenerator::getBaseAddress(CatScopeID scopeId, LLVMCompileTimeContext* context)
{
	llvm::Value* parentObjectAddress = nullptr;
	if (context->catContext->isStaticScope(scopeId))
	{
		unsigned char* object = context->catContext->getScopeObject(scopeId);
		parentObjectAddress = llvm::ConstantInt::get(LLVMJit::get().getContext(), llvm::APInt(sizeof(std::uintptr_t) * 8, (uint64_t)reinterpret_cast<std::uintptr_t>(object), false));
	}
	else
	{
		//Get the CatRuntimeContext argument from the current function
		assert(context->currentFunction != nullptr);
		assert(context->currentFunction->arg_size() > 0);
		llvm::Argument* argument = context->currentFunction->arg_begin();
		if (context->currentFunction->arg_size() > 0 && context->currentFunction->hasAttribute(1, llvm::Attribute::StructRet))
		{
			argument = context->currentFunction->arg_begin() + 1;
		}
		assert(argument->getName() == "RuntimeContext");
		assert(argument->getType() == LLVMTypes::pointerType);
		llvm::Value* scopeIdValue = context->helper->createConstant((int)scopeId);
		llvm::Value* address = address = helper->createCall(context, &LLVMCatIntrinsics::getScopePointerFromContext, {argument, scopeIdValue}, "getScopePointerFromContext"); 
		assert(address != nullptr);
		parentObjectAddress = helper->convertToIntPtr(address, "CustomThis_IntPtr");
	}

	return parentObjectAddress;
}


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generateMemberFunctionCall(MemberFunctionInfo* memberFunction, CatTypedExpression* base, 
																		 const std::vector<const CatTypedExpression*>& arguments, 
																		 LLVMCompileTimeContext* context)
{
	CatGenericType& returnType = memberFunction->returnType;
	llvm::Value* baseObject = generate(base, context);
	if (memberFunction->isDeferredFunctionCall())
	{
		//We must first get the deferred base.
		DeferredMemberFunctionInfo* deferredFunctionInfo = static_cast<DeferredMemberFunctionInfo*>(memberFunction);
		baseObject = deferredFunctionInfo->baseMember->generateDereferenceCode(baseObject, context);
	}
	//If the member function call returns an object by value, we must allocate the object on the stack.
	llvm::Value* returnedObjectAllocation = helper->generateFunctionCallReturnValueAllocation(returnType, memberFunction->memberFunctionName, context);
	
	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		std::vector<llvm::Value*> argumentList;
		llvm::Value* functionThis = compileContext->helper->convertToPointer(baseObject, memberFunction->memberFunctionName + "_This_Ptr");
		argumentList.push_back(functionThis);
		std::vector<llvm::Type*> argumentTypes;
		argumentTypes.push_back(LLVMTypes::pointerType);
		MemberFunctionCallData callData = memberFunction->getFunctionAddress();
		if (!callData.makeDirectCall)
		{
			//Add an argument that contains a pointer to a MemberFunctionInfo object.
			llvm::Value* memberFunctionAddressValue = compileContext->helper->createIntPtrConstant(callData.functionInfoStructAddress, "MemberFunctionInfo_IntPtr");
			llvm::Value* memberFunctionPtrValue = compileContext->helper->convertToPointer(memberFunctionAddressValue, "MemberFunctionInfo_Ptr");
			argumentList.push_back(memberFunctionPtrValue);
			argumentTypes.push_back(LLVMTypes::pointerType);
		}
		helper->generateFunctionCallArgumentEvalatuation(arguments, memberFunction->argumentTypes, argumentList, argumentTypes, this, context);

		uintptr_t functionAddress = callData.makeDirectCall ? callData.memberFunctionAddress : callData.staticFunctionAddress;

		llvm::Type* returnLLVMType = context->helper->toLLVMType(returnType);
		if (!returnType.isStringType() && !returnType.isContainerType() && !returnType.isReflectableObjectType())
		{
			llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
			llvm::CallInst* call = static_cast<llvm::CallInst*>(compileContext->helper->createCall(functionType, functionAddress, argumentList, base->getType().toString() + "." + memberFunction->memberFunctionName));
			if (Configuration::useThisCall && callData.makeDirectCall)
			{
				call->setCallingConv(llvm::CallingConv::X86_ThisCall);
			}
			return static_cast<llvm::Value*>(call);
		}
		else if (returnType.isStringType() || returnType.isReflectableObjectType())
		{
			auto sretTypeInsertPoint = argumentTypes.begin();
			if (!Configuration::sretBeforeThis && callData.makeDirectCall)
			{
				sretTypeInsertPoint++;
			}
			llvm::Type* returnLLVMTypePtr = returnLLVMType;
			if (!returnType.isStringType())
			{
				returnLLVMTypePtr = LLVMTypes::pointerType;
			}
			argumentTypes.insert(sretTypeInsertPoint, returnLLVMTypePtr);

			llvm::FunctionType* functionType = llvm::FunctionType::get(LLVMTypes::voidType, argumentTypes, false);
			auto sretInsertPoint = argumentList.begin();
			if (!Configuration::sretBeforeThis && callData.makeDirectCall)
			{
				sretInsertPoint++;
			}
			argumentList.insert(sretInsertPoint, returnedObjectAllocation);
			llvm::CallInst* call = static_cast<llvm::CallInst*>(compileContext->helper->createCall(functionType, functionAddress, argumentList, base->getType().toString() + "." + memberFunction->memberFunctionName));
			if (Configuration::useThisCall && callData.makeDirectCall)
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
	};

	llvm::Type* resultType = helper->toLLVMType(returnType);
	if (returnType.isReflectableObjectType())
	{
		resultType = resultType->getPointerTo();
	}
	if (returnType.isStringType() || returnType.isReflectableObjectType())
	{
		auto codeGenIfNull = [=](LLVMCompileTimeContext* context)
		{
			//Default construct the object.
			if (returnType.isStringType())
			{
				helper->createCall(context, &LLVMCatIntrinsics::stringEmptyConstruct, {returnedObjectAllocation}, "stringDefaultConstruct");
			}
			else
			{
				assert(returnType.isConstructible());
				llvm::Constant* typeInfoConstant = helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(returnType.getObjectType()), Tools::append(returnType.getObjectType()->getTypeName(), "_typeInfo"));
				llvm::Value* typeInfoConstantAsIntPtr = helper->convertToPointer(typeInfoConstant, Tools::append(returnType.getObjectType()->getTypeName(), "_typeInfoPtr"));
				helper->createCall(context, &LLVMCatIntrinsics::placementConstructType, {returnedObjectAllocation, typeInfoConstantAsIntPtr}, Tools::append(returnType.getObjectType()->getTypeName(), "_defaultConstructor"));
			}
			return returnedObjectAllocation;
		};
		return helper->createOptionalNullCheckSelect(baseObject, notNullCodeGen, codeGenIfNull, context);
	}
	else
	{
		return helper->createOptionalNullCheckSelect(baseObject, notNullCodeGen, resultType, context);
	}
}


void LLVMCodeGenerator::initContext(LLVMCompileTimeContext* context)
{
	context->helper = helper.get();
	context->currentDyLib = dylib;
}


void LLVMCodeGenerator::createNewModule(LLVMCompileTimeContext* context)
{
	currentModule.reset(new llvm::Module(context->catContext->getContextName(), LLVMJit::get().getContext()));
	currentModule->setTargetTriple(LLVMJit::get().getTargetMachine().getTargetTriple().str());
	currentModule->setDataLayout(LLVMJit::get().getDataLayout());

	// Create a new pass manager attached to it.
	passManager = std::make_unique<llvm::legacy::FunctionPassManager>(currentModule.get());

	// Do simple "peephole" optimizations and bit-twiddling optzns.
	passManager->add(llvm::createInstructionCombiningPass());
	// Reassociate expressions.
	passManager->add(llvm::createReassociatePass());
	// Eliminate Common SubExpressions.
	passManager->add(llvm::createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	passManager->add(llvm::createCFGSimplificationPass());

	passManager->doInitialization();

	helper->setCurrentModule(currentModule.get());

}


std::string LLVMCodeGenerator::getNextFunctionName(LLVMCompileTimeContext * context)
{
	return Tools::append("expression_", context->catContext->getContextName(), "_", context->catContext->getNextFunctionIndex());
}


llvm::Function* LLVMCodeGenerator::verifyAndOptimizeFunction(llvm::Function* function)
{
	if (!llvm::verifyFunction(*function, &llvm::outs()))
	{
		passManager->run(*function);
		if constexpr (Configuration::dumpFunctionIR)
		{
			function->dump();
		}		
		return function;
	}
	else
	{
#ifdef _DEBUG
		function->dump();
#endif
		assert(false);
		LLVMJit::logError("Function contains errors.");
		return nullptr;
	}
}


llvm::Expected<llvm::JITEvaluatedSymbol> LLVMCodeGenerator::findSymbol(const std::string& name, llvm::orc::JITDylib& dyLib) const
{
	return executionSession->lookup({&dyLib}, mangler->operator()(name));
}


llvm::JITTargetAddress LLVMCodeGenerator::getSymbolAddress(const std::string& name, llvm::orc::JITDylib& dyLib) const
{
	return llvm::cantFail(findSymbol(name, dyLib)).getAddress();
}

std::unique_ptr<LLVMMemoryManager> LLVMCodeGenerator::memoryManager = std::make_unique<LLVMMemoryManager>();