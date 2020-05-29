/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCodeGenerator.h"
#include "jitcat/CatASTNodes.h"
#include "jitcat/CatLib.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeMemberFunctionInfo.h"
#include "jitcat/ErrorContext.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMJit.h"
#include "jitcat/LLVMMemoryManager.h"
#include "jitcat/LLVMPreGeneratedExpression.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/StaticMemberInfo.h"
#include "jitcat/StringConstantPool.h"

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
	executionSession(std::make_unique<llvm::orc::ExecutionSession>()),
	currentModule(std::make_unique<llvm::Module>("JitCat", LLVMJit::get().getContext())),
	builder(std::make_unique<llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>>(LLVMJit::get().getContext())),
	objectLinkLayer(std::make_unique<llvm::orc::RTDyldObjectLinkingLayer>(*executionSession.get(),
															[]() {	return memoryManager->createExpressionAllocator();})),
	mangler(std::make_unique<llvm::orc::MangleAndInterner>(*executionSession, LLVMJit::get().getDataLayout())),
	compileLayer(std::make_unique<llvm::orc::IRCompileLayer>(*executionSession.get(), *(objectLinkLayer.get()), std::make_unique<llvm::orc::ConcurrentIRCompiler>(LLVMJit::get().getTargetMachineBuilder()))),
	helper(std::make_unique<LLVMCodeGeneratorHelper>(builder.get(), currentModule.get()))
{
	llvm::orc::SymbolMap intrinsicSymbols;
	runtimeLibraryDyLib = &executionSession->createJITDylib("runtimeLibrary");

	llvm::JITSymbolFlags functionFlags;
	functionFlags |= llvm::JITSymbolFlags::Callable;
	functionFlags |= llvm::JITSymbolFlags::Exported;
	functionFlags |= llvm::JITSymbolFlags::Absolute;


	intrinsicSymbols[executionSession->intern("fmodf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&fmodf), functionFlags);
	intrinsicSymbols[executionSession->intern("_fmod")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&fmodl), functionFlags);
	double(*fmodPtr)(double, double) = &fmod;
	intrinsicSymbols[executionSession->intern("fmod")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(fmodPtr), functionFlags);

	intrinsicSymbols[executionSession->intern("sinf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&sinf), functionFlags);
	intrinsicSymbols[executionSession->intern("_sin")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&sinl), functionFlags);
	double(*sinPtr)(double) = &sin;
	intrinsicSymbols[executionSession->intern("sin")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(sinPtr), functionFlags);

	intrinsicSymbols[executionSession->intern("cosf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&cosf), functionFlags);
	intrinsicSymbols[executionSession->intern("_cos")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&cosl), functionFlags);
	double(*cosPtr)(double) = &cos;
	intrinsicSymbols[executionSession->intern("cos")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(cosPtr), functionFlags);

	intrinsicSymbols[executionSession->intern("log10f")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&log10f), functionFlags);
	intrinsicSymbols[executionSession->intern("_log10")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&log10l), functionFlags);
	double(*log10Ptr)(double) = &log10;
	intrinsicSymbols[executionSession->intern("log10")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(log10Ptr), functionFlags);

	intrinsicSymbols[executionSession->intern("logf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&logf), functionFlags);
	intrinsicSymbols[executionSession->intern("_log")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&logl), functionFlags);
	double(*logPtr)(double) = &log;
	intrinsicSymbols[executionSession->intern("log")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(logPtr), functionFlags);

	intrinsicSymbols[executionSession->intern("expf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&expf), functionFlags);
	intrinsicSymbols[executionSession->intern("_exp")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&expl), functionFlags);
	double(*expPtr)(double) = &exp;
	intrinsicSymbols[executionSession->intern("exp")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(expPtr), functionFlags);

	intrinsicSymbols[executionSession->intern("powf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&powf), functionFlags);
	intrinsicSymbols[executionSession->intern("_pow")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&powl), functionFlags);
	double(*powPtr)(double, double) = &pow;
	intrinsicSymbols[executionSession->intern("pow")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(powPtr), functionFlags);

	intrinsicSymbols[executionSession->intern("ceilf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&ceilf), functionFlags);
	intrinsicSymbols[executionSession->intern("_ceil")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&ceill), functionFlags);
	double(*ceilPtr)(double) = &ceil;
	intrinsicSymbols[executionSession->intern("ceil")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(ceilPtr), functionFlags);

	intrinsicSymbols[executionSession->intern("floorf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&floorf), functionFlags);
	intrinsicSymbols[executionSession->intern("_floor")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&floorl), functionFlags);
	double(*floorPtr)(double) = &floor;
	intrinsicSymbols[executionSession->intern("floor")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(floorPtr), functionFlags);
	

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


void LLVMCodeGenerator::generate(const AST::CatSourceFile* sourceFile, LLVMCompileTimeContext* context)
{
	initContext(context);
	createNewModule(context);
	CatScopeID staticScopeId = context->catContext->addScope(sourceFile->getCustomType(), sourceFile->getScopeObjectInstance(), true);
	assert(staticScopeId == sourceFile->getScopeId());
	for (auto& iter : sourceFile->getFunctionDefinitions())
	{
		generate(iter, context);
	}
	for (auto& iter : sourceFile->getClassDefinitions())
	{
		generate(iter, context);
	}
	context->catContext->removeScope(staticScopeId);
	llvm::cantFail(compileLayer->add(*dylib, llvm::orc::ThreadSafeModule(std::move(currentModule), LLVMJit::get().getThreadSafeContext())));
	link(sourceFile->getCustomType());
}


llvm::Value* LLVMCodeGenerator::generate(const CatTypedExpression* expression, LLVMCompileTimeContext* context)
{
	initContext(context);
	switch (expression->getNodeType())
	{
		case CatASTNodeType::AssignmentOperator:			return generate(static_cast<const CatAssignmentOperator*>(expression), context);	
		case CatASTNodeType::BuiltInFunctionCall:			return generate(static_cast<const CatBuiltInFunctionCall*>(expression), context);		
		case CatASTNodeType::IndirectionConversion:			return generate(static_cast<const CatIndirectionConversion*>(expression), context);
		case CatASTNodeType::InfixOperator:					return generate(static_cast<const CatInfixOperator*>(expression), context);	
		case CatASTNodeType::Literal:						return generate(static_cast<const CatLiteral*>(expression), context);			
		case CatASTNodeType::LLVMPreGeneratedExpression:	return static_cast<const LLVMPreGeneratedExpression*>(expression)->getValue();
		case CatASTNodeType::MemberAccess:					return generate(static_cast<const CatMemberAccess*>(expression), context);		
		case CatASTNodeType::MemberFunctionCall:			return generate(static_cast<const CatMemberFunctionCall*>(expression), context);
		case CatASTNodeType::PrefixOperator:				return generate(static_cast<const CatPrefixOperator*>(expression), context);	
		case CatASTNodeType::ScopeRoot:						return generate(static_cast<const CatScopeRoot*>(expression), context);		
		case CatASTNodeType::StaticFunctionCall:			return generate(static_cast<const CatStaticFunctionCall*>(expression), context);
		case CatASTNodeType::StaticMemberAccess:			return generate(static_cast<const CatStaticMemberAccess*>(expression), context);
		case CatASTNodeType::ReturnStatement:				return generate(static_cast<const CatReturnStatement*>(expression), context);
		default:											assert(false);
	}
	assert(false);
	return nullptr;
}


llvm::Function* LLVMCodeGenerator::generateExpressionFunction(const CatTypedExpression* expression, LLVMCompileTimeContext* context, const std::string& name)
{
	initContext(context);
	
	CatGenericType expressionType = expression->getType();

	//Generate the prototype for the function. No code is yet associated with the function at this time.
	llvm::Function* function = generateFunctionPrototype(name, false, expressionType, {TypeTraits<CatRuntimeContext*>::toGenericType()}, {"RuntimeContext"});
	
	//Now, generate code for the function
	context->currentFunction = function;
	
	//Function entry block
	llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
	builder->SetInsertPoint(&function->getEntryBlock());

	//Generate code for the expression.
	llvm::Value* expressionValue = generate(expression, context);

	//Return the expression value
	generateFunctionReturn(expressionType, expressionValue, function, context);

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
	llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
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
	//To silence unused variable warning in release builds.
	(void)function;
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
	//To silence unused variable warning in release builds.
	(void)function;
	assert(function != nullptr);
	llvm::cantFail(compileLayer->add(*dylib, llvm::orc::ThreadSafeModule(std::move(currentModule), LLVMJit::get().getThreadSafeContext())));
	return (intptr_t)getSymbolAddress(functionName.c_str(), *dylib);
}


const CatGenericType& getFPType(const CatGenericType& inType)
{
	if (inType.isDoubleType())
	{
		return inType;
	}
	else
	{
		return CatGenericType::floatType;
	}
}


llvm::Value* LLVMCodeGenerator::generate(const CatBuiltInFunctionCall* functionCall, LLVMCompileTimeContext* context)
{
	CatArgumentList* arguments = functionCall->getArgumentList();
	switch (functionCall->getFunctionType())
	{
		case CatBuiltInFunctionType::ToVoid:	generate(arguments->getArgument(0), context); return nullptr;
		case CatBuiltInFunctionType::ToInt:		return helper->convertType(generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), CatGenericType::intType, context);
		case CatBuiltInFunctionType::ToDouble:	return helper->convertType(generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), CatGenericType::doubleType, context);
		case CatBuiltInFunctionType::ToFloat:	return helper->convertType(generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), CatGenericType::floatType, context);
		case CatBuiltInFunctionType::ToBool:	return helper->convertType(generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), CatGenericType::boolType, context);
		case CatBuiltInFunctionType::Select:
		{
			llvm::Value* conditionValue = helper->convertType(generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), CatGenericType::boolType, context);
			llvm::Value* trueValue = generate(arguments->getArgument(1), context);
			llvm::Value* falseValue = helper->convertType(generate(arguments->getArgument(2), context), arguments->getArgument(2)->getType(), arguments->getArgument(1)->getType(), context);
			return builder->CreateSelect(conditionValue, trueValue, falseValue);
		}
		case CatBuiltInFunctionType::Abs:
		{
			if (arguments->getArgumentType(0).isDoubleType() || arguments->getArgumentType(0).isFloatType())
			{
				return helper->callIntrinsic(llvm::Intrinsic::fabs, arguments->getArgumentType(0), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);
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
		case CatBuiltInFunctionType::Log10:	return helper->callIntrinsic(llvm::Intrinsic::log10, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);
		case CatBuiltInFunctionType::Sqrt:	return helper->callIntrinsic(llvm::Intrinsic::sqrt, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);
		case CatBuiltInFunctionType::Ln:	return helper->callIntrinsic(llvm::Intrinsic::log, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);
		case CatBuiltInFunctionType::Exp:	return helper->callIntrinsic(llvm::Intrinsic::exp, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);
		case CatBuiltInFunctionType::Pow:
		{
			return helper->callIntrinsic(llvm::Intrinsic::pow, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), generate(arguments->getArgument(1), context), arguments->getArgument(1)->getType(), context);
		}
		case CatBuiltInFunctionType::Sin:		return helper->callIntrinsic(llvm::Intrinsic::sin, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);
		case CatBuiltInFunctionType::Cos:		return helper->callIntrinsic(llvm::Intrinsic::cos, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);
		case CatBuiltInFunctionType::Ceil:		return helper->callIntrinsic(llvm::Intrinsic::ceil, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);
		case CatBuiltInFunctionType::Floor:		return helper->callIntrinsic(llvm::Intrinsic::floor, getFPType(arguments->getArgumentType(0)), generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), context);

		case CatBuiltInFunctionType::Tan:		return generateFPMath("tan", &tanf, &tan, arguments, context);
		case CatBuiltInFunctionType::Asin:		return generateFPMath("asin", &asinf, &asin, arguments, context);
		case CatBuiltInFunctionType::Acos:		return generateFPMath("acos", &acosf, &acos, arguments, context);
		case CatBuiltInFunctionType::Atan:		return generateFPMath("atan", &atanf, &atan, arguments, context);
		case CatBuiltInFunctionType::Sinh:		return generateFPMath("sinh", &sinhf, &sinh, arguments, context);
		case CatBuiltInFunctionType::Cosh:		return generateFPMath("cosh", &coshf, &cosh, arguments, context);
		case CatBuiltInFunctionType::Tanh:		return generateFPMath("tanh", &tanhf, &tanh, arguments, context);
		case CatBuiltInFunctionType::Asinh:		return generateFPMath("asinh", &asinhf, &asinh, arguments, context);
		case CatBuiltInFunctionType::Acosh:		return generateFPMath("acosh", &acoshf, &acosh, arguments, context);
		case CatBuiltInFunctionType::Atanh:		return generateFPMath("atanh", &atanhf, &atanh, arguments, context);
		case CatBuiltInFunctionType::Atan2:		return generateFPMath("atan2", &atan2f, &atan2, arguments, context);
		case CatBuiltInFunctionType::Hypot:		return generateFPMath("hypot", &hypotf, &hypot, arguments, context);
		case CatBuiltInFunctionType::Cap:
		{
			llvm::Value* value = generate(arguments->getArgument(0), context);
			llvm::Value* minValue = generate(arguments->getArgument(1), context);
			llvm::Value* maxValue = generate(arguments->getArgument(2), context);
			if (arguments->getArgumentType(0).isFloatType() || arguments->getArgumentType(0).isDoubleType())
			{
				llvm::Value* convertedMin = helper->convertType(minValue, arguments->getArgument(1)->getType(), arguments->getArgumentType(0), context);
				llvm::Value* convertedMax = helper->convertType(maxValue, arguments->getArgument(2)->getType(), arguments->getArgumentType(0), context);
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
				llvm::Value* convertedMin = helper->convertType(minValue, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
				llvm::Value* convertedMax = helper->convertType(maxValue, arguments->getArgument(2)->getType(), CatGenericType::intType, context);
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
			if (arguments->getArgumentType(0).isFloatType() || arguments->getArgumentType(0).isDoubleType())
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, arguments->getArgument(1)->getType(), arguments->getArgumentType(0), context);
				llvm::Value* lessThan = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OLT, value1, convertedValue2);
				return builder->CreateSelect(lessThan, value1, convertedValue2);
			}
			else if (arguments->getArgumentType(0).isIntType())
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
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
			if (arguments->getArgumentType(0).isFloatType() || arguments->getArgumentType(0).isDoubleType())
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, arguments->getArgument(1)->getType(), arguments->getArgumentType(0), context);
				llvm::Value* greaterThan = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OGT, value1, convertedValue2);
				return builder->CreateSelect(greaterThan, value1, convertedValue2);
			}
			else if (arguments->getArgumentType(0).isIntType())
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
				llvm::Value* greaterThan = builder->CreateICmp(llvm::CmpInst::Predicate::ICMP_SGT, value1, convertedValue2);
				return builder->CreateSelect(greaterThan, value1, convertedValue2);
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}
		case CatBuiltInFunctionType::ToString:
		{
			return helper->convertType(generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), CatGenericType::stringWeakPtrType, context);
		}
		case CatBuiltInFunctionType::ToPrettyString:
		{
			if (arguments->getArgumentType(0).isIntType())
			{
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::intToPrettyString, {generate(arguments->getArgument(0), context)}, "intToPrettyString");
			}
			else
			{
				return helper->convertType(generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), CatGenericType::stringWeakPtrType, context);
			}
		}
		case CatBuiltInFunctionType::ToFixedLengthString:
		{
			llvm::Value* number = helper->convertType(generate(arguments->getArgument(0), context), arguments->getArgument(0)->getType(), CatGenericType::intType, context);
			llvm::Value* length = helper->convertType(generate(arguments->getArgument(1), context), arguments->getArgument(1)->getType(), CatGenericType::intType, context);
			return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::intToFixedLengthString, {number, length}, "intToFixedLengthString");
		}
		case CatBuiltInFunctionType::Random:
		{
			return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::getRandomFloat, {}, "getRandomFloat");
		}
		case CatBuiltInFunctionType::RandomRange:
		{
			llvm::Value* left = generate(arguments->getArgument(0), context);
			llvm::Value* right = generate(arguments->getArgument(1), context);
			if (arguments->getArgumentType(0).isBoolType()
				&& arguments->getArgumentType(1).isBoolType())
			{
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::getRandomBoolean, {left, right}, "getRandomBoolean");
			}
			else if (arguments->getArgumentType(0).isIntType()
					 && arguments->getArgumentType(1).isIntType())
			{
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::getRandomInt, {left, right}, "getRandomInt");
			}
			else if (arguments->getArgumentType(0).isDoubleType() || arguments->getArgumentType(1).isDoubleType())
			{
				llvm::Value* leftDouble = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::doubleType, context);
				llvm::Value* rightDouble = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::doubleType, context);
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::getRandomDoubleRange, {leftDouble, rightDouble}, "getRandomDouble");
			}
			else
			{
				llvm::Value* leftFloat = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::floatType, context);
				llvm::Value* rightFloat = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::floatType, context);
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::getRandomFloatRange, {leftFloat, rightFloat}, "getRandomFloat");
			}
		}
		case CatBuiltInFunctionType::Round:
		{
			llvm::Value* left = generate(arguments->getArgument(0), context);
			llvm::Value* right = generate(arguments->getArgument(1), context);
			if (arguments->getArgumentType(0).isDoubleType())
			{
				llvm::Value* leftDouble = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::doubleType, context);
				llvm::Value* rightInt = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::roundDouble, {leftDouble, rightInt}, "roundDouble");
			}
			else
			{
				llvm::Value* leftFloat = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::floatType, context);
				llvm::Value* rightInt = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::roundFloat, {leftFloat, rightInt}, "roundFloat");
			}
		}
		case CatBuiltInFunctionType::StringRound:
		{
			llvm::Value* left = generate(arguments->getArgument(0), context);
			llvm::Value* right = generate(arguments->getArgument(1), context);
			if (arguments->getArgumentType(0).isDoubleType())
			{
				llvm::Value* leftDouble = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::doubleType, context);
				llvm::Value* rightInt = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::roundDoubleToString, {leftDouble, rightInt}, "roundDoubleToString");
			}
			else
			{
				llvm::Value* leftFloat = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::floatType, context);
				llvm::Value* rightInt = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::roundFloatToString, {leftFloat, rightInt}, "roundFloatToString");
			}
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
				return helper->createOptionalNullCheckSelect(generatedExpression, [&](LLVMCompileTimeContext* context)
				{
					//Dereference the pointer
					return builder->CreateLoad(generatedExpression, generatedExpression->getType()->getPointerElementType(), "Dereference");				
				}, generatedExpression->getType()->getPointerElementType(), context);
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
			return helper->createOptionalNullCheckSelect(generatedExpression, [&](LLVMCompileTimeContext* context)
			{
				//Dereference the pointer
				return builder->CreateLoad(generatedExpression, generatedExpression->getType()->getPointerElementType(), "Dereference");				
			}, generatedExpression->getType()->getPointerElementType(), context);
			
		}
		case IndirectionConversionMode::DereferencePointerToPointerTwice:
		{
			//Check that we are dealing with a pointer to a pointer
			assert(generatedExpression->getType()->isPointerTy());
			assert(generatedExpression->getType()->getPointerElementType()->isPointerTy());

			return helper->createOptionalNullCheckSelect(generatedExpression, [&](LLVMCompileTimeContext* context)
			{
				//Dereference the pointer
				llvm::Value* value = builder->CreateLoad(generatedExpression, generatedExpression->getType()->getPointerElementType(), "Dereference");			
				return helper->createOptionalNullCheckSelect(value, [&](LLVMCompileTimeContext* context)
					{
						if (expressionToConvert->getType().getPointeeType()->getPointeeType()->isBasicType())
						{
							value = builder->CreateLoad(value, value->getType()->getPointerElementType(), "Dereference");				
						}
						return value;
					}, value->getType()->getPointerElementType(), context);
			}, generatedExpression->getType()->getPointerElementType(), context);

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

	const CatGenericType& leftType = infixOperator->getLeft()->getType();
	const CatGenericType& rightType = infixOperator->getRight()->getType();

	assert(left != nullptr && right != nullptr);

	if (oper == CatInfixOperatorType::LogicalOr)
	{
		return builder->CreateOr(helper->convertType(left, leftType, CatGenericType::boolType, context), 
								 helper->convertType(right, rightType, CatGenericType::boolType, context), "or");
	}
	else if (oper == CatInfixOperatorType::LogicalAnd)
	{
		return builder->CreateAnd(helper->convertType(left, leftType, CatGenericType::boolType, context), 
								  helper->convertType(right, rightType, CatGenericType::boolType, context), "and");
	}
	else if (left->getType() != right->getType())
	{
		//Operators must always operate on identical types.
		//If types are different, we must first convert them.
		if (leftType.isStringType()
			|| rightType.isStringType())
		{
			left = helper->convertType(left, leftType, CatGenericType::stringWeakPtrType, context);
			right = helper->convertType(right, rightType, CatGenericType::stringWeakPtrType, context);
		}
		else if (leftType.isDoubleType()
				|| rightType.isDoubleType())
		{
			left = helper->convertType(left, leftType, CatGenericType::doubleType, context);
			right = helper->convertType(right, rightType, CatGenericType::doubleType, context);
		}
		else if (leftType.isFloatType()
				|| rightType.isFloatType())
		{
			left = helper->convertType(left, leftType, CatGenericType::floatType, context);
			right = helper->convertType(right, rightType, CatGenericType::floatType, context);
		}
		else
		{
			//left and right are ints or booleans, booleans will be promoted to ints
			left = helper->convertType(left, leftType, CatGenericType::intType, context);
			right = helper->convertType(right, rightType, CatGenericType::intType, context);
		}
	}
	if (leftType.isFloatType() || leftType.isDoubleType())
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
					llvm::Value* zeroConstant = helper->createZeroInitialisedConstant(helper->toLLVMType(leftType));
					return builder->CreateSelect(builder->CreateFCmpUEQ(right, zeroConstant), zeroConstant, builder->CreateFDiv(left, right, "divided"));			
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
					llvm::Value* zeroConstant = helper->createZeroInitialisedConstant(helper->toLLVMType(leftType));
					return builder->CreateSelect(builder->CreateFCmpUEQ(right, zeroConstant), zeroConstant, builder->CreateFRem(left, right, "divided"));
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
			default:										assert(false);
		}
	}
	else if (leftType.isIntType())
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
					llvm::Value* zeroConstant = helper->createZeroInitialisedConstant(helper->toLLVMType(leftType));
					return builder->CreateSelect(builder->CreateICmpEQ(right, zeroConstant), zeroConstant, builder->CreateSDiv(left, right, "divided"));					 
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
					llvm::Value* zeroConstant = helper->createZeroInitialisedConstant(helper->toLLVMType(leftType));
					return builder->CreateSelect(builder->CreateICmpEQ(right, zeroConstant), zeroConstant, builder->CreateSRem(left, right, "modulo"));			
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
			default:										assert(false);
		}
	}
	else if (leftType.isBoolType())
	{
		switch (oper)
		{
			case CatInfixOperatorType::Equals:				return builder->CreateICmpEQ(left, right, "equal");
			case CatInfixOperatorType::NotEquals:			return builder->CreateICmpNE(left, right, "notEqual");		
			default:										assert(false);
		}
	}
	else if (leftType.isStringType())
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
				return helper->createOptionalNullCheckSelect(stringNullCheck, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createIntrinsicCall(compileContext, &LLVMCatIntrinsics::stringAppend, {left, right}, "stringAppend");}, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createNullPtrConstant(LLVMTypes::pointerType);}, context);
			}
			case CatInfixOperatorType::Equals:
			{
				return helper->createOptionalNullCheckSelect(stringNullCheck, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createIntrinsicCall(compileContext, &LLVMCatIntrinsics::stringEquals, {left, right}, "stringEquals");}, LLVMTypes::boolType, context);  
			}
			case CatInfixOperatorType::NotEquals:		
			{
				return helper->createOptionalNullCheckSelect(stringNullCheck, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createIntrinsicCall(compileContext, &LLVMCatIntrinsics::stringNotEquals, {left, right}, "stringNotEquals");}, [&](LLVMCompileTimeContext* compileContext){return compileContext->helper->createConstant(true);}, context);
			}
			default:	assert(false);
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
	else if (literalType.isDoubleType())	return helper->createConstant(std::any_cast<double>(literal->getValue()));
	else if (literalType.isFloatType())		return helper->createConstant(std::any_cast<float>(literal->getValue()));
	else if (literalType.isBoolType())		return helper->createConstant(std::any_cast<bool>(literal->getValue()));
	else if (literalType.isStringPtrType())
	{
		Configuration::CatString* stringPtr = std::any_cast<Configuration::CatString*>(literal->getValue());
		return helper->createPtrConstant(reinterpret_cast<std::uintptr_t>(stringPtr), "stringLiteralAddress");
	}
	else if (literalType.isStringValueType())
	{
		Configuration::CatString stringValue = std::any_cast<Configuration::CatString>(literal->getValue());
		const Configuration::CatString* stringPtr = StringConstantPool::getString(stringValue);
		return helper->createPtrConstant(reinterpret_cast<std::uintptr_t>(stringPtr), "stringLiteralAddress");
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
	if (base != nullptr)
	{
		return memberAccess->getMemberInfo()->generateDereferenceCode(base, context);
	}
	else
	{
		//This means that the member is a function parameter
		const std::string& memberName = memberAccess->getMemberName();
		for (auto iter = context->currentFunction->arg_begin(); iter != context->currentFunction->arg_end(); ++iter)
		{
			if (Tools::equalsWhileIgnoringCase(memberName, iter->getName()))
			{
				return iter;
			}
		}
		assert(false);
		return nullptr;
	}
	
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
	

	llvm::Value* returnAllocation = helper->generateFunctionCallReturnValueAllocation(returnType, staticFunctionCall->getFunctionName(), context);
	if (returnAllocation != nullptr)
	{
		argumentList.push_back(returnAllocation);
		argumentTypes.push_back(returnAllocation->getType());
	}
	helper->generateFunctionCallArgumentEvalatuation(expressionArguments, staticFunctionCall->getExpectedParameterTypes(), argumentList, argumentTypes, this, context);

	return helper->generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, staticFunctionCall->getFunctionAddress(), staticFunctionCall->getFunctionName(), returnAllocation);
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
	const CatGenericType& rightType = prefixOperator->getRHS()->getType();
	if (!(rightType.isIntType() || rightType.isBoolType() || rightType.isFloatType() || rightType.isDoubleType()))
	{
		assert(false);
		return LLVMJit::logError("ERROR: Type not yet supported for prefix operators.");
	}
	if (prefixOperator->getOperator() == CatPrefixOperator::Operator::Not)
	{
		return builder->CreateNot(helper->convertType(right, prefixOperator->getRHS()->getType(), CatGenericType::boolType, context), "not");
	}
	else if (prefixOperator->getOperator() == CatPrefixOperator::Operator::Minus)
	{
		if (rightType.isFloatType() || rightType.isDoubleType())
		{
			return builder->CreateFNeg(right, "negative");
		}
		else
		{
			return builder->CreateNeg(helper->convertType(right, prefixOperator->getRHS()->getType(), CatGenericType::intType, context), "negative");
		}
	}
	else
	{
		assert(false);
		return LLVMJit::logError("ERROR: Operator not implemented.");
	}
}


llvm::Value* LLVMCodeGenerator::generate(const CatScopeRoot* scopeRoot, LLVMCompileTimeContext* context)
{
	return getBaseAddress(scopeRoot->getScopeId(), context);
}


void jitcat::LLVM::LLVMCodeGenerator::generate(const AST::CatScopeBlock* scopeBlock, LLVMCompileTimeContext* context)
{
	if (scopeBlock != nullptr)
	{
		CatScopeID blockScopeId = context->catContext->addScope(scopeBlock->getCustomType(), nullptr, false);
		assert(blockScopeId == scopeBlock->getScopeId());
		for (auto& iter : scopeBlock->getStatements())
		{
			generate(iter.get(), context);
		}
		context->catContext->removeScope(blockScopeId);
	}
}


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generate(const AST::CatReturnStatement* returnStatement, LLVMCompileTimeContext* context)
{
	generate(context->currentFunctionDefinition->getEpilogBlock(), context);
	llvm::Value* returnValue = nullptr;
	if (!returnStatement->getType().isVoidType())
	{
		returnValue = generate(returnStatement->getReturnExpression(), context);
	}
	generateFunctionReturn(context->currentFunctionDefinition->getReturnTypeNode()->getType(), returnValue, context->currentFunction, context);
	return nullptr;
}


void LLVMCodeGenerator::generate(const AST::CatStatement* statement, LLVMCompileTimeContext* context)
{
	switch (statement->getNodeType())
	{
		case CatASTNodeType::ForLoop:
		case CatASTNodeType::IfStatement:
		case CatASTNodeType::ScopeBlock:
		case CatASTNodeType::VariableDeclaration:
		assert(false); return;
		default:
			generate(static_cast<const CatTypedExpression*>(statement), context);
	}
}


void LLVMCodeGenerator::generate(const AST::CatDefinition* definition, LLVMCompileTimeContext* context)
{
	initContext(context);

	switch (definition->getNodeType())
	{
		case CatASTNodeType::ClassDefinition:
		case CatASTNodeType::InheritanceDefinition:
		case CatASTNodeType::FunctionDefinition:	
		case CatASTNodeType::VariableDefinition:
		default: assert(false);
	}
	
}


void LLVMCodeGenerator::generate(const AST::CatClassDefinition* classDefinition, LLVMCompileTimeContext* context)
{
	assert(context->currentLib != nullptr);
	const CatClassDefinition* previousClass = context->currentClass;
	context->currentClass = classDefinition;
	ErrorContext errorContext(context->catContext, classDefinition->getClassName());
	for (auto& iter: classDefinition->getClassDefinitions())
	{
		generate(iter, context);
	}
	CatScopeID classScopeId = context->catContext->addScope(classDefinition->getCustomType(), nullptr, false);
	assert(classScopeId == classDefinition->getScopeId());
	for (auto& iter: classDefinition->getFunctionDefinitions())
	{
		generate(iter, context);
	}
	CatFunctionDefinition* constructorDefinition = classDefinition->getFunctionDefinitionByName("__init");
	if (constructorDefinition != nullptr)
	{
		generate(constructorDefinition, context);
	}
	context->currentClass = previousClass;
	context->catContext->removeScope(classScopeId);
}


llvm::Function* LLVMCodeGenerator::generate(const AST::CatFunctionDefinition* functionDefinition, LLVMCompileTimeContext* context)
{
	assert(context->currentLib != nullptr);
	const std::string& name = functionDefinition->getMangledFunctionName();
	CatGenericType returnType = functionDefinition->getReturnTypeNode()->getType();
	std::vector<CatGenericType> parameterTypes;
	std::vector<std::string> parameterNames;
	for (int i = 0; i < functionDefinition->getNumParameters(); i++)
	{
		if (!functionDefinition->getParameterType(i).isReflectableObjectType())
		{
			parameterTypes.push_back(functionDefinition->getParameterType(i));
		}
		else
		{
			parameterTypes.push_back(functionDefinition->getParameterType(i).toPointer(TypeOwnershipSemantics::Value, true, false));
		}
		parameterNames.push_back(functionDefinition->getParameterName(i));
	}

	bool isThisCall = context->currentClass != nullptr;

	llvm::Function* function = generateFunctionPrototype(name, isThisCall, returnType, parameterTypes, parameterNames);
	
	context->currentFunction = function;
	//Function entry block
	llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
	builder->SetInsertPoint(&function->getEntryBlock());

	context->currentFunctionDefinition = functionDefinition;

	CatScopeID classScopeId = InvalidScopeID;
	if (isThisCall)
	{
		classScopeId = context->currentClass->getScopeId();
		if (returnType.isReflectableObjectType())
		{
			auto argIter = function->arg_begin();
			if (Configuration::sretBeforeThis)
			{
				argIter++;
			}
			context->scopeValues[classScopeId] = argIter;
		}
		else
		{
			context->scopeValues[classScopeId] = function->arg_begin();
		}
	}

	CatScopeBlock* scopeBlock = functionDefinition->getScopeBlock();
	CatScopeID parametersScopeId = InvalidScopeID;
	if (functionDefinition->getNumParameters() > 0)
	{
		parametersScopeId = context->catContext->addScope(functionDefinition->getParametersType(), nullptr, false);
		context->scopeValues[parametersScopeId] = nullptr;
	}
	assert(parametersScopeId == functionDefinition->getScopeId());
	generate(scopeBlock, context);
	if (!functionDefinition->getAllControlPathsReturn() && returnType.isVoidType())
	{
		generateFunctionReturn(returnType, nullptr, function, context);
	}
	
	context->catContext->removeScope(parametersScopeId);

	if (classScopeId != InvalidScopeID)
	{
		context->scopeValues.erase(classScopeId);
	}

	if (parametersScopeId != InvalidScopeID)
	{
		context->scopeValues.erase(parametersScopeId);
	}

	context->currentFunction = nullptr;

	context->blockDestructorGenerators.clear();

	//Verify the correctness of the function and execute optimization passes.
	return verifyAndOptimizeFunction(function);
}


llvm::Value* LLVMCodeGenerator::generateAssign(const CatAssignableExpression* expression, llvm::Value* rValue, LLVMCompileTimeContext* context)
{
	context->helper = helper.get();
	if (expression->getType().isPointerToReflectableObjectType()
		&& expression->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value)
	{
		//Need to call operator=
		SearchFunctionSignature signature("=", {expression->getType().removeIndirection().toPointer(TypeOwnershipSemantics::Weak)});
		MemberFunctionInfo* functionInfo = expression->getType().getPointeeType()->getObjectType()->getMemberFunctionInfo(signature);
		if (functionInfo != nullptr)
		{
			LLVMPreGeneratedExpression preGenerated(rValue, expression->getType().removeIndirection().toPointer());
			return generateMemberFunctionCall(functionInfo, expression, {&preGenerated}, context);
		}
		assert(false);
	}
	else
	{
		switch (expression->getNodeType())
		{
			case CatASTNodeType::StaticIdentifier:	return generateAssign(static_cast<const CatStaticIdentifier*>(expression), rValue, context);
			case CatASTNodeType::MemberAccess:		return generateAssign(static_cast<const CatMemberAccess*>(expression), rValue, context);
			default:									assert(false);
		}
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


llvm::Value* LLVMCodeGenerator::generateFPMath(const char* name, float(*floatVariant)(float), double(*doubleVariant)(double), 
											   const AST::CatArgumentList* argumentList, LLVMCompileTimeContext* context)
{
	if (argumentList->getArgumentType(0).isDoubleType())
	{
		return helper->createIntrinsicCall(context, doubleVariant, { generate(argumentList->getArgument(0), context)}, name);
	}
	else
	{
		return helper->createIntrinsicCall(context, floatVariant, { helper->convertType(generate(argumentList->getArgument(0), context), argumentList->getArgumentType(0), CatGenericType::floatType, context) }, name);
	}
}


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generateFPMath(const char* name, float(*floatVariant)(float, float), double(*doubleVariant)(double, double), 
															 const AST::CatArgumentList* argumentList, LLVMCompileTimeContext* context)
{
	if (argumentList->getArgumentType(0).isDoubleType() || argumentList->getArgumentType(1).isDoubleType())
	{
		llvm::Value* x = helper->convertType(generate(argumentList->getArgument(0), context), argumentList->getArgument(0)->getType(), CatGenericType::doubleType, context);
		llvm::Value* y = helper->convertType(generate(argumentList->getArgument(1), context), argumentList->getArgument(1)->getType(), CatGenericType::doubleType, context);
		return helper->createIntrinsicCall(context, doubleVariant, { x, y }, name);
	}
	else
	{
		llvm::Value* x = helper->convertType(generate(argumentList->getArgument(0), context), argumentList->getArgument(0)->getType(), CatGenericType::floatType, context);
		llvm::Value* y = helper->convertType(generate(argumentList->getArgument(1), context), argumentList->getArgument(1)->getType(), CatGenericType::floatType, context);
		return helper->createIntrinsicCall(context, floatVariant, { x, y }, name);
	}
}


llvm::Value* LLVMCodeGenerator::getBaseAddress(CatScopeID scopeId, LLVMCompileTimeContext* context)
{
	llvm::Value* parentObjectAddress = nullptr;
	if (auto scopeIter = context->scopeValues.find(scopeId); scopeIter != context->scopeValues.end())
	{
		return scopeIter->second;
	}
	else if (context->catContext->isStaticScope(scopeId))
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
		llvm::Value* address = address = helper->createIntrinsicCall(context, &LLVMCatIntrinsics::getScopePointerFromContext, {argument, scopeIdValue}, "getScopePointerFromContext"); 
		assert(address != nullptr);
		parentObjectAddress = helper->convertToIntPtr(address, "CustomThis_IntPtr");
	}

	return parentObjectAddress;
}


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generateMemberFunctionCall(MemberFunctionInfo* memberFunction, const CatTypedExpression* base, 
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
		MemberFunctionCallData callData = memberFunction->getFunctionAddress();
		assert(callData.functionAddress != 0);
		std::vector<llvm::Value*> argumentList;
		llvm::Value* functionThis = compileContext->helper->convertToPointer(baseObject, memberFunction->memberFunctionName + "_This_Ptr");
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
		helper->generateFunctionCallArgumentEvalatuation(arguments, memberFunction->argumentTypes, argumentList, argumentTypes, this, context);

		uintptr_t functionAddress = callData.functionAddress;

		llvm::Type* returnLLVMType = context->helper->toLLVMType(returnType);
		if (!returnType.isReflectableObjectType())
		{
			if (callData.callType == MemberFunctionCallType::PseudoMemberCall)
			{
				return helper->generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, functionAddress, memberFunction->memberFunctionName, returnedObjectAllocation);
			}
			else
			{
				llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
				llvm::CallInst* call = static_cast<llvm::CallInst*>(compileContext->helper->createCall(functionType, functionAddress, argumentList, base->getType().toString() + "." + memberFunction->memberFunctionName));
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
			return helper->generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, functionAddress, memberFunction->memberFunctionName, returnedObjectAllocation);
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
			llvm::CallInst* call = static_cast<llvm::CallInst*>(compileContext->helper->createCall(functionType, functionAddress, argumentList, base->getType().toString() + "." + memberFunction->memberFunctionName));
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
	};

	llvm::Type* resultType = helper->toLLVMType(returnType);
	if (returnType.isReflectableObjectType())
	{
		resultType = resultType->getPointerTo();
	}
	if (returnType.isReflectableObjectType())
	{
		auto codeGenIfNull = [=](LLVMCompileTimeContext* context)
		{
			assert(returnType.isConstructible());
			llvm::Constant* typeInfoConstant = helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(returnType.getObjectType()), Tools::append(returnType.getObjectType()->getTypeName(), "_typeInfo"));
			llvm::Value* typeInfoConstantAsIntPtr = helper->convertToPointer(typeInfoConstant, Tools::append(returnType.getObjectType()->getTypeName(), "_typeInfoPtr"));
			helper->createIntrinsicCall(context, &LLVMCatIntrinsics::placementConstructType, {returnedObjectAllocation, typeInfoConstantAsIntPtr}, Tools::append(returnType.getObjectType()->getTypeName(), "_defaultConstructor"));
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


llvm::Function* jitcat::LLVM::LLVMCodeGenerator::generateFunctionPrototype(const std::string& functionName, bool isThisCall, const CatGenericType& returnType, const std::vector<CatGenericType>& parameterTypes, const std::vector<std::string>& parameterNames)
{
	assert(parameterTypes.size() == parameterNames.size());

	//Define the parameters for the function.
	//If the function returns an object by value, the object is returned through an object pointer parameter and the function returns void.
	//The object pointer should point to pre-allocated memory where the returned object will be constructed.
	std::vector<llvm::Type*> parameters;
	llvm::FunctionType* functionType = nullptr;

	llvm::Type* functionReturnType = nullptr;

	if (isThisCall)
	{
		parameters.push_back(LLVMTypes::pointerType);
	}
	if (returnType.isReflectableObjectType())
	{
		parameters.push_back(LLVMTypes::pointerType);
		functionReturnType = LLVMTypes::voidType;
	}
	else
	{
		functionReturnType = helper->toLLVMType(returnType);
	}
	for (std::size_t i = 0; i < parameterTypes.size(); i++)
	{
		parameters.push_back(helper->toLLVMType(parameterTypes[i]));
	}
	functionType = llvm::FunctionType::get(functionReturnType, parameters, false);		

	//Create the function signature. No code is yet associated with the function at this time.
	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, functionName.c_str(), currentModule.get());

	//Attributes and names for the parameters can now be set on the function signature.
	//When returning a string, the StructRet attribute is set to indicate that the parameter is used for returning a structure by value.
	llvm::Argument* currentArg = function->arg_begin();
	if (returnType.isReflectableObjectType())
	{
		if (!isThisCall || Configuration::sretBeforeThis)
		{
			currentArg->setName(Tools::append(returnType.getObjectTypeName(), "__sret"));
			function->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
			function->addParamAttr(0, llvm::Attribute::AttrKind::NoAlias);
		}
		else
		{
			currentArg->setName("__this");
		}
		currentArg++;
		if (isThisCall && Configuration::sretBeforeThis)
		{
			currentArg->setName("__this");
			currentArg++;
		}
		else if (isThisCall)
		{
			currentArg->setName(Tools::append(returnType.getObjectTypeName(), "__sret"));
			currentArg++;
			function->addParamAttr(1, llvm::Attribute::AttrKind::StructRet);
			function->addParamAttr(1, llvm::Attribute::AttrKind::NoAlias);
		}
	}
	else if (isThisCall)
	{
		currentArg->setName("__this");
		currentArg++;
	}
	for (std::size_t i = 0; i < parameterNames.size(); i++)
	{
		currentArg->setName(parameterNames[i]);
		currentArg++;
	}
	return function;
}


void LLVMCodeGenerator::generateFunctionReturn(const CatGenericType& returnType, llvm::Value* expressionValue, llvm::Function* function, LLVMCompileTimeContext* context)
{
	//If the expression returns a string or a reflectable object, copy construct it into the StructRet parameter and return void.
	//If it is some other type, just return the value.
	if (returnType.isReflectableObjectType())
	{
		llvm::Constant* typeInfoConstant = helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(returnType.getObjectType()), Tools::append(returnType.toString(), "_typeInfo"));
		llvm::Value* typeInfoConstantAsIntPtr = helper->convertToPointer(typeInfoConstant, Tools::append(returnType.toString(), "_typeInfoPtr"));
		assert(returnType.isCopyConstructible());
		llvm::Value* castPointer = builder->CreatePointerCast(expressionValue, LLVMTypes::pointerType, Tools::append(returnType.toString(), "_ObjectPointerCast"));

		llvm::Argument* sretArgument = function->arg_begin();
		if (context->currentClass != nullptr && !Configuration::sretBeforeThis)
		{
			++sretArgument;
		}
		helper->createOptionalNullCheckSelect(castPointer, 
			[&](LLVMCompileTimeContext* context)
			{
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::placementCopyConstructType, {sretArgument, castPointer, typeInfoConstantAsIntPtr}, Tools::append(returnType.toString(), "_copyConstructor"));
			},
			[&](LLVMCompileTimeContext* context)
			{
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::placementConstructType, {sretArgument, typeInfoConstantAsIntPtr},  Tools::append(returnType.toString(), "_defaultConstructor"));
			}, context);

		helper->generateBlockDestructors(context);
		builder->CreateRetVoid();
	}
	else
	{
		helper->generateBlockDestructors(context);
		builder->CreateRet(expressionValue);
	}
}


void LLVMCodeGenerator::link(CustomTypeInfo* customType)
{
	for (auto& iter : customType->getTypes())
	{
		link(static_cast<CustomTypeInfo*>(iter.second));
	}
	for (auto& iter : customType->getMemberFunctions())
	{
		const std::string& mangledName = static_cast<CustomTypeMemberFunctionInfo*>(iter.second.get())->functionDefinition->getMangledFunctionName();
		static_cast<CustomTypeMemberFunctionInfo*>(iter.second.get())->nativeAddress = (intptr_t)getSymbolAddress(mangledName, *dylib);
	}
}


std::unique_ptr<LLVMMemoryManager> LLVMCodeGenerator::memoryManager = std::make_unique<LLVMMemoryManager>();