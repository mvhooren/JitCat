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
#include "jitcat/ExpressionHelperFunctions.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMJit.h"
#include "jitcat/LLVMTargetConfig.h"
#include "jitcat/LLVMTargetConfigOptions.h"
#include "jitcat/LLVMMemoryManager.h"
#include "jitcat/LLVMPrecompilationContext.h"
#include "jitcat/LLVMPreGeneratedExpression.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/StaticMemberInfo.h"
#include "jitcat/StringConstantPool.h"

#include <functional>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/IR/Argument.h>
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
#include <llvm/Transforms/Utils.h>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;


struct ScopeChecker
{
	ScopeChecker(LLVMCompileTimeContext* context):
		numScopeValues(context->scopeValues.size()),
		numScopes(context->catContext->getNumScopes()),
		context(context)
	{}
	~ScopeChecker()
	{
		assert(numScopeValues == context->scopeValues.size());
		assert(numScopes == context->catContext->getNumScopes());
	}

	std::size_t numScopeValues;
	int numScopes;
	LLVMCompileTimeContext* context;
};
#ifdef NDEBUG
#define ScopeCheck(expression) ((void)0)
#else
#define ScopeCheck(expression) ScopeChecker checker(expression)
#endif

LLVMCodeGenerator::LLVMCodeGenerator(const std::string& name, const LLVMTargetConfig* targetConfig):
	targetConfig(targetConfig),
	currentModule(std::make_unique<llvm::Module>("JitCat", LLVMJit::get().getContext())),
	builder(std::make_unique<llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>>(LLVMJit::get().getContext()))
{
	helper = std::make_unique<LLVMCodeGeneratorHelper>(this);
	if (targetConfig->isJITTarget)
	{
		executionSession = std::make_unique<llvm::orc::ExecutionSession>(LLVMJit::get().getSymbolStringPool());
		objectLinkLayer = std::make_unique<llvm::orc::RTDyldObjectLinkingLayer>(*executionSession.get(),
																				[]() {	return memoryManager->createExpressionAllocator();});
		mangler = std::make_unique<llvm::orc::MangleAndInterner>(*executionSession, targetConfig->getDataLayout());
		compileLayer = std::make_unique<llvm::orc::IRCompileLayer>(*executionSession.get(), *(objectLinkLayer.get()), std::make_unique<llvm::orc::ConcurrentIRCompiler>(*targetConfig->getTargetMachineBuilder()));
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

		intrinsicSymbols[executionSession->intern("exp2f")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&exp2f), functionFlags);
		intrinsicSymbols[executionSession->intern("_exp2")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&exp2l), functionFlags);
		double(*exp2Ptr)(double) = &exp2;
		intrinsicSymbols[executionSession->intern("exp2")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(exp2Ptr), functionFlags);

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

		intrinsicSymbols[executionSession->intern("memset")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&memset), functionFlags);
		intrinsicSymbols[executionSession->intern("memcpy")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&memcpy), functionFlags);
	

		llvm::cantFail(runtimeLibraryDyLib->define(llvm::orc::absoluteSymbols(intrinsicSymbols)));

		dylib = &executionSession->createJITDylib(Tools::append(name, "_", 0));
		dylib->addToSearchOrder(*runtimeLibraryDyLib);

		if (targetConfig->enableSymbolSearchWorkaround)
		{
			objectLinkLayer->setAutoClaimResponsibilityForObjectSymbols(true);
			objectLinkLayer->setOverrideObjectFlagsWithResponsibilityFlags(true);
		}
	}

	std::string targetTriple = targetConfig->getTargetMachine().getTargetTriple().str();
	currentModule->setTargetTriple(targetTriple);
	currentModule->setDataLayout(targetConfig->getDataLayout());

	// Create a new pass manager attached to it.
	passManager = std::make_unique<llvm::legacy::FunctionPassManager>(currentModule.get());

	createOptimisationPasses(passManager.get());

}


LLVMCodeGenerator::~LLVMCodeGenerator()
{
}


void LLVMCodeGenerator::generate(const AST::CatSourceFile* sourceFile, LLVMCompileTimeContext* context)
{
	initContext(context);
	createNewModule(context);
	CatScopeID staticScopeId = context->catContext->addStaticScope(sourceFile->getCustomType(), sourceFile->getScopeObjectInstance(), sourceFile->getFileName());
	assert(staticScopeId == sourceFile->getScopeId());
	for (auto& iter : sourceFile->getClassDefinitions())
	{
		iter->setDylib(dylib);
	}
	for (auto& iter : sourceFile->getFunctionDefinitions())
	{
		generate(iter, context);
	}
	for (auto& iter : sourceFile->getClassDefinitions())
	{
		generate(iter, context);
	}
	context->catContext->removeScope(staticScopeId);
	if (!context->isPrecompilationContext)
	{
		llvm::cantFail(compileLayer->add(*dylib, llvm::orc::ThreadSafeModule(std::move(currentModule), LLVMJit::get().getThreadSafeContext())));
		link(sourceFile->getCustomType());
	}
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


llvm::Function* LLVMCodeGenerator::generateExpressionFunction(const CatTypedExpression* expression, LLVMCompileTimeContext* context, const std::string& name, bool generateThisCall)
{
	initContext(context);
	
	CatGenericType expressionType = expression->getType();

	//Generate the prototype for the function. No code is yet associated with the function at this time.
	llvm::Function* function = generateFunctionPrototype(name, generateThisCall, expressionType, {TypeTraits<CatRuntimeContext*>::toGenericType()}, {"RuntimeContext"});
	
	function->addFnAttr(llvm::Attribute::UWTable);
	function->addFnAttr(llvm::Attribute::NoInline);
	//Now, generate code for the function
	context->currentFunction = function;
	
	//Function entry block
	llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
	builder->SetInsertPoint(&function->getEntryBlock());

	//Generate code for the expression.
	llvm::Value* expressionValue = generate(expression, context);

	//Return the expression value
	generateFunctionReturn(expressionType, expressionValue, function, context, generateThisCall);

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
	std::vector<llvm::Type*> parameters = {targetConfig->getLLVMTypes().pointerType, helper->toLLVMType(expression->getType())};
	llvm::FunctionType* functionType = llvm::FunctionType::get(targetConfig->getLLVMTypes().voidType, parameters, false);		
	//Create the function signature. No code is yet associated with the function at this time.
	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, name.c_str(), currentModule.get());
	function->setCallingConv(targetConfig->getOptions().defaultLLVMCallingConvention);
	
	if (targetConfig->getOptions().explicitEnableTailCalls)		function->addFnAttr("disable-tail-calls", "false");
	if (targetConfig->getOptions().nonLeafFramePointer)			function->addFnAttr("frame-pointer", "non-leaf");
	if (targetConfig->getOptions().strongStackProtection)		
	{
		function->addFnAttr(llvm::Attribute::StackProtectStrong);
		function->addFnAttr("stack-protector-buffer-size", "8");
	}

	function->addFnAttr(llvm::Attribute::UWTable);
	function->addFnAttr(llvm::Attribute::NoInline);
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


intptr_t LLVMCodeGenerator::generateAndGetFunctionAddress(const CatTypedExpression* expression, const std::string& expressionStr, 
														  const CatGenericType& expectedType, LLVMCompileTimeContext* context,
														  bool generateThisCall)
{
	initContext(context);
	createNewModule(context);
	std::string functionName = ExpressionHelperFunctions::getUniqueExpressionFunctionName(expressionStr, context->catContext, false, expectedType);

	if (auto lookupResult = executionSession->lookup({dylib}, mangler->operator()(functionName)))
	{
		return (intptr_t)lookupResult.get().getAddress();
	}
	else
	{
		llvm::handleAllErrors(lookupResult.takeError(), [](const llvm::orc::SymbolsNotFound& err){});
		llvm::Function* function = generateExpressionFunction(expression, context, functionName, generateThisCall);
		//To silence unused variable warning in release builds.
		(void)function;
		assert(function != nullptr);
		llvm::cantFail(compileLayer->add(*dylib, llvm::orc::ThreadSafeModule(std::move(currentModule), LLVMJit::get().getThreadSafeContext())));
		return (intptr_t)getSymbolAddress(functionName.c_str(), *dylib);
	}
}


intptr_t LLVMCodeGenerator::generateAndGetAssignFunctionAddress(const CatAssignableExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, LLVMCompileTimeContext* context)
{
	initContext(context);
	createNewModule(context);
	std::string functionName = ExpressionHelperFunctions::getUniqueExpressionFunctionName(expressionStr, context->catContext, true, expectedType);

	if (auto lookupResult = executionSession->lookup({dylib}, mangler->operator()(functionName)))
	{
		return (intptr_t)lookupResult.get().getAddress();
	}
	else
	{	
		llvm::handleAllErrors(lookupResult.takeError(), [](const llvm::orc::SymbolsNotFound& err){});
		llvm::Function* function = generateExpressionAssignFunction(expression, context, functionName);
		//To silence unused variable warning in release builds.
		(void)function;
		assert(function != nullptr);
		llvm::cantFail(compileLayer->add(*dylib, llvm::orc::ThreadSafeModule(std::move(currentModule), LLVMJit::get().getThreadSafeContext())));
		return (intptr_t)getSymbolAddress(functionName.c_str(), *dylib);
	}
}


void LLVMCodeGenerator::emitModuleToObjectFile(const std::string& objectFileName)
{
	llvm::legacy::PassManager pass;
	auto FileType = llvm::CGFT_ObjectFile;

	std::error_code EC;
	llvm::raw_fd_ostream dest(objectFileName, EC, llvm::sys::fs::OF_None);

	if (EC) {
	  llvm::errs() << "Could not open file: " << EC.message();
	  return;
	}

	if (targetConfig->getTargetMachine().addPassesToEmitFile(pass, dest, nullptr, FileType)) 
	{
	  llvm::errs() << "TargetMachine can't emit a file of this type";
	  return;
	}

	pass.run(*currentModule);
	dest.flush();
}


llvm::Function* LLVMCodeGenerator::generateExpressionSymbolEnumerationFunction(const std::unordered_map<std::string, llvm::Function*>& symbols)
{
	const std::string enumerationFunctionName = "_jc_enumerate_expressions";
	llvm::Type* functionReturnType = targetConfig->getLLVMTypes().voidType;

	std::vector<llvm::Type*> callBackParameters = {targetConfig->getLLVMTypes().pointerType, targetConfig->getLLVMTypes().pointerType};
	llvm::FunctionType* callBackType = llvm::FunctionType::get(functionReturnType, callBackParameters, false);		

	std::vector<llvm::Type*> parameters = {callBackType->getPointerTo()};
	llvm::FunctionType* functionType = llvm::FunctionType::get(functionReturnType, parameters, false);		
	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, enumerationFunctionName.c_str(), currentModule.get());
	function->setCallingConv(targetConfig->getOptions().defaultLLVMCallingConvention);

	llvm::FunctionCallee callee(callBackType, function->getArg(0));

	llvm::BasicBlock::Create(LLVMJit::get().getContext(), "entry", function);
	builder->SetInsertPoint(&function->getEntryBlock());

	for (auto iter : symbols)
	{
		llvm::Constant* zeroTerminatedString = helper->createZeroTerminatedStringConstant(iter.first);
		llvm::Value* functionPtr = helper->convertToPointer(iter.second, "functionPtr");
		llvm::CallInst* callInst = builder->CreateCall(callee, {zeroTerminatedString, functionPtr});
		callInst->setCallingConv(targetConfig->getOptions().defaultLLVMCallingConvention);
	}
	builder->CreateRetVoid();
	return verifyAndOptimizeFunction(function);
}


llvm::Function* LLVMCodeGenerator::generateGlobalVariablesEnumerationFunction(const std::unordered_map<std::string, llvm::GlobalVariable*>& globals)
{
	return helper->generateGlobalVariableEnumerationFunction(globals, "_jc_enumerate_global_variables");
}


llvm::Function* LLVMCodeGenerator::generateLinkedFunctionsEnumerationFunction(const std::unordered_map<std::string, llvm::GlobalVariable*>& functionPointers)
{
	return helper->generateGlobalVariableEnumerationFunction(functionPointers, "_jc_enumerate_linked_functions");
}


llvm::Function* LLVMCodeGenerator::generateStringPoolInitializationFunction(const std::unordered_map<std::string, llvm::GlobalVariable*>& stringGlobals)
{
	return helper->generateGlobalVariableEnumerationFunction(stringGlobals, "_jc_initialize_string_pool");
}


llvm::Function* LLVMCodeGenerator::generateJitCatABIVersionFunction()
{
	return helper->generateConstIntFunction(Configuration::jitcatABIVersion, "_jc_get_jitcat_abi_version");
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
			return builder->CreateSelect(builder->CreateTrunc(conditionValue, targetConfig->getLLVMTypes().bool1Type), trueValue, falseValue);
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

		case CatBuiltInFunctionType::Tan:		return generateFPMath("tanf", &tanf, "tan", &tan, arguments, context);
		case CatBuiltInFunctionType::Asin:		return generateFPMath("asinf", &asinf, "asin", &asin, arguments, context);
		case CatBuiltInFunctionType::Acos:		return generateFPMath("acosf", &acosf, "acos", &acos, arguments, context);
		case CatBuiltInFunctionType::Atan:		return generateFPMath("atanf", &atanf, "atan", &atan, arguments, context);
		case CatBuiltInFunctionType::Sinh:		return generateFPMath("sinhf", &sinhf, "sinh", &sinh, arguments, context);
		case CatBuiltInFunctionType::Cosh:		return generateFPMath("coshf", &coshf, "cosh", &cosh, arguments, context);
		case CatBuiltInFunctionType::Tanh:		return generateFPMath("tanhf", &tanhf, "tanh", &tanh, arguments, context);
		case CatBuiltInFunctionType::Asinh:		return generateFPMath("asinhf", &asinhf, "asinh", &asinh, arguments, context);
		case CatBuiltInFunctionType::Acosh:		return generateFPMath("acoshf", &acoshf, "acosh", &acosh, arguments, context);
		case CatBuiltInFunctionType::Atanh:		return generateFPMath("atanhf", &atanhf, "atanh", &atanh, arguments, context);
		case CatBuiltInFunctionType::Atan2:		return generateFPMath("atan2f", &atan2f, "atan2", &atan2, arguments, context);
		case CatBuiltInFunctionType::Hypot:		return generateFPMath("hypotf", &hypotf, "hypot", &hypot, arguments, context);
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
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::intToPrettyString, {generate(arguments->getArgument(0), context)}, "intToPrettyString", false);
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
			return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::intToFixedLengthString, {number, length}, "intToFixedLengthString", false);
		}
		case CatBuiltInFunctionType::Random:
		{
			return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_getRandomFloat, {}, "_jc_getRandomFloat", true);
		}
		case CatBuiltInFunctionType::RandomRange:
		{
			llvm::Value* left = generate(arguments->getArgument(0), context);
			llvm::Value* right = generate(arguments->getArgument(1), context);
			if (arguments->getArgumentType(0).isBoolType()
				&& arguments->getArgumentType(1).isBoolType())
			{
				return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_getRandomBoolean, {left, right}, "_jc_getRandomBoolean", true);
			}
			else if (arguments->getArgumentType(0).isIntType()
					 && arguments->getArgumentType(1).isIntType())
			{
				return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_getRandomInt, {left, right}, "_jc_getRandomInt", true);
			}
			else if (arguments->getArgumentType(0).isDoubleType() || arguments->getArgumentType(1).isDoubleType())
			{
				llvm::Value* leftDouble = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::doubleType, context);
				llvm::Value* rightDouble = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::doubleType, context);
				return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_getRandomDoubleRange, {leftDouble, rightDouble}, "_jc_getRandomDoubleRange", true);
			}
			else
			{
				llvm::Value* leftFloat = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::floatType, context);
				llvm::Value* rightFloat = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::floatType, context);
				return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_getRandomFloatRange, {leftFloat, rightFloat}, "_jc_getRandomFloatRange", true);
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
				return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_roundDouble, {leftDouble, rightInt}, "_jc_roundDouble", true);
			}
			else
			{
				llvm::Value* leftFloat = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::floatType, context);
				llvm::Value* rightInt = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
				return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_roundFloat, {leftFloat, rightInt}, "_jc_roundFloat", true);
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
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::roundDoubleToString, {leftDouble, rightInt}, "roundDoubleToString", false);
			}
			else
			{
				llvm::Value* leftFloat = helper->convertType(left, arguments->getArgument(0)->getType(), CatGenericType::floatType, context);
				llvm::Value* rightInt = helper->convertType(right, arguments->getArgument(1)->getType(), CatGenericType::intType, context);
				return helper->createIntrinsicCall(context, &LLVMCatIntrinsics::roundFloatToString, {leftFloat, rightInt}, "roundFloatToString", false);
			}
		}
		default:
		{
			assert(false);
			return nullptr;
		}
	}
}


llvm::Value* LLVMCodeGenerator::generate(const CatIndirectionConversion* indirectionConversion, LLVMCompileTimeContext* context)
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

	CatGenericType leftType = infixOperator->getLeft()->getType();
	CatGenericType rightType = infixOperator->getRight()->getType();

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
			leftType = CatGenericType::stringWeakPtrType;
			rightType = CatGenericType::stringWeakPtrType;
		}
		else if (leftType.isDoubleType()
				|| rightType.isDoubleType())
		{
			left = helper->convertType(left, leftType, CatGenericType::doubleType, context);
			right = helper->convertType(right, rightType, CatGenericType::doubleType, context);
			leftType = CatGenericType::doubleType;
			rightType = CatGenericType::doubleType;
		}
		else if (leftType.isFloatType()
				|| rightType.isFloatType())
		{
			left = helper->convertType(left, leftType, CatGenericType::floatType, context);
			right = helper->convertType(right, rightType, CatGenericType::floatType, context);
			leftType = CatGenericType::floatType;
			rightType = CatGenericType::floatType;
		}
		else
		{
			//left and right are ints or booleans, booleans will be promoted to ints
			left = helper->convertType(left, leftType, CatGenericType::intType, context);
			right = helper->convertType(right, rightType, CatGenericType::intType, context);
			leftType = CatGenericType::intType;
			rightType = CatGenericType::intType;
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
			case CatInfixOperatorType::Greater:				return booleanCast(builder->CreateFCmpUGT(left, right, "greater"));
			case CatInfixOperatorType::Smaller:				return booleanCast(builder->CreateFCmpULT(left, right, "smaller"));		
			case CatInfixOperatorType::GreaterOrEqual:		return booleanCast(builder->CreateFCmpUGE(left, right, "greaterOrEqual"));	
			case CatInfixOperatorType::SmallerOrEqual:		return booleanCast(builder->CreateFCmpULE(left, right, "lessOrEqual"));	
			case CatInfixOperatorType::Equals:				return booleanCast(builder->CreateFCmpUEQ(left, right, "equal"));			
			case CatInfixOperatorType::NotEquals:			return booleanCast(builder->CreateFCmpUNE(left, right, "notEqual"));
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
			case CatInfixOperatorType::Greater:				return booleanCast(builder->CreateICmpSGT(left, right, "greater"));		
			case CatInfixOperatorType::Smaller:				return booleanCast(builder->CreateICmpSLT(left, right, "smaller"));		
			case CatInfixOperatorType::GreaterOrEqual:		return booleanCast(builder->CreateICmpSGE(left, right, "greaterOrEqual"));	
			case CatInfixOperatorType::SmallerOrEqual:		return booleanCast(builder->CreateICmpSLE(left, right, "smallerOrEqual"));	
			case CatInfixOperatorType::Equals:				return booleanCast(builder->CreateICmpEQ(left, right, "equal"));
			case CatInfixOperatorType::NotEquals:			return booleanCast(builder->CreateICmpNE(left, right, "notEqual"));		
			default:										assert(false);
		}
	}
	else if (leftType.isBoolType())
	{
		switch (oper)
		{
			case CatInfixOperatorType::Equals:				return booleanCast(builder->CreateICmpEQ(left, right, "equal"));
			case CatInfixOperatorType::NotEquals:			return booleanCast(builder->CreateICmpNE(left, right, "notEqual"));		
			default:										assert(false);
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
	if	(literalType.isCharType())			return helper->createConstant(std::any_cast<char>(literal->getValue()));
	else if (literalType.isBoolType())		return helper->createConstant(std::any_cast<bool>(literal->getValue()));
	else if	(literalType.isUCharType())		return helper->createConstant(std::any_cast<unsigned char>(literal->getValue()));
	else if	(literalType.isIntType())		return helper->createConstant(std::any_cast<int>(literal->getValue()));
	else if	(literalType.isUIntType())		return helper->createConstant(std::any_cast<unsigned int>(literal->getValue()));
	else if	(literalType.isInt64Type())		return helper->createConstant(std::any_cast<int64_t>(literal->getValue()));
	else if	(literalType.isUInt64Type())	return helper->createConstant(std::any_cast<uint64_t>(literal->getValue()));
	else if (literalType.isDoubleType())	return helper->createConstant(std::any_cast<double>(literal->getValue()));
	else if (literalType.isFloatType())		return helper->createConstant(std::any_cast<float>(literal->getValue()));
	else if (literalType.isVector4fType())	return helper->createConstant(std::any_cast<std::array<float, 4>>(literal->getValue()));
	else if (literalType.isStringPtrType())
	{
		Configuration::CatString* stringPtr = std::any_cast<Configuration::CatString*>(literal->getValue());
		if (!context->isPrecompilationContext)
		{
			return helper->createPtrConstant(context, reinterpret_cast<std::uintptr_t>(stringPtr), "stringLiteralAddress");
		}
		else
		{
			llvm::GlobalVariable* stringPtrPtr = std::static_pointer_cast<LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalString(*stringPtr, context);
			return helper->loadPointerAtAddress(stringPtrPtr, "stringLiteralAddress");
		}
	}
	else if (literalType.isStringValueType())
	{
		Configuration::CatString stringValue = std::any_cast<Configuration::CatString>(literal->getValue());

		if (!context->isPrecompilationContext)
		{
			const Configuration::CatString* stringPtr = StringConstantPool::getString(stringValue);
			return helper->createPtrConstant(context, reinterpret_cast<std::uintptr_t>(stringPtr), "stringLiteralAddress");
		}
		else
		{
			llvm::GlobalVariable* stringPtrPtr = std::static_pointer_cast<LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalString(stringValue, context);
			return helper->loadPointerAtAddress(stringPtrPtr, "stringLiteralAddress");
		}
	}
	else if (literalType.isPointerToReflectableObjectType())
	{
		if (!context->isPrecompilationContext)
		{
			uintptr_t pointerConstant = literalType.getRawPointer(literal->getValue());
			llvm::Value* reflectableAddress = helper->createIntPtrConstant(context, pointerConstant, "literalObjectAddress");
			return builder->CreateIntToPtr(reflectableAddress, targetConfig->getLLVMTypes().pointerType);
		}
		else
		{
			assert(false && "pointer literals are not allowed when pre-compiling");
			return nullptr;
		}
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
	return helper->generateMemberFunctionCall(memberFunctionCall->getMemberFunctionInfo(), memberFunctionCall->getBase(), 
											  expressionArguments, memberFunctionCall->getArgumentsToCheckForNull(), context);
}


llvm::Value* LLVMCodeGenerator::generate(const AST::CatStaticFunctionCall* staticFunctionCall, LLVMCompileTimeContext* context)
{
	const CatArgumentList* arguments = staticFunctionCall->getArguments();
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
	int argumentsOffset = (int)argumentList.size();
	helper->generateFunctionCallArgumentEvalatuation(expressionArguments, staticFunctionCall->getExpectedParameterTypes(), argumentList, argumentTypes, this, context);
	llvm::Value* argumentsNullCheck = helper->generateFunctionCallArgumentNullChecks(argumentList, staticFunctionCall->getArgumentsToCheckForNull(), argumentsOffset, this, context);
	helper->defineWeakSymbol(context, staticFunctionCall->getFunctionAddress(), staticFunctionCall->getMangledFunctionName(targetConfig->sretBeforeThis), false);
	if (argumentsNullCheck != nullptr)
	{
		//There are arguments that had to be checked for null
		auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
		{
			return helper->generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, staticFunctionCall->getMangledFunctionName(targetConfig->sretBeforeThis), 
													  staticFunctionCall->getFunctionName(), returnAllocation, false, staticFunctionCall->getFunctionNeverReturnsNull());
		};
		return helper->createOptionalNullCheckSelect(argumentsNullCheck, notNullCodeGen, returnType, returnAllocation, context);
	}
	else
	{
		
		return helper->generateStaticFunctionCall(returnType, argumentList, argumentTypes, context, staticFunctionCall->getMangledFunctionName(targetConfig->sretBeforeThis), 
												  staticFunctionCall->getFunctionName(), returnAllocation, false, staticFunctionCall->getFunctionNeverReturnsNull());
	}
}


llvm::Value* LLVMCodeGenerator::generate(const AST::CatStaticMemberAccess* staticIdentifier, LLVMCompileTimeContext* context)
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
		return builder->CreateXor(helper->convertType(right, prefixOperator->getRHS()->getType(), CatGenericType::boolType, context), helper->createConstant(true), "not");
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


void LLVMCodeGenerator::generate(const AST::CatScopeBlock* scopeBlock, LLVMCompileTimeContext* context)
{
	if (scopeBlock != nullptr)
	{
		std::size_t blockDestructorsSize = context->blockDestructorGenerators.size();
		const CatScopeBlock* previousScopeBlock = context->currentScope;
		context->currentScope = scopeBlock;
		CatScopeID blockScopeId = context->catContext->addDynamicScope(scopeBlock->getCustomType(), nullptr);
		assert(blockScopeId == scopeBlock->getScopeId());
		if (scopeBlock->getCustomType()->getTypeSize() > 0)
		{
			llvm::Value* scopeAlloc = helper->createObjectAllocA(context, "scope_locals", CatGenericType(scopeBlock->getCustomType(), true, false), false);
			context->scopeValues[blockScopeId] = scopeAlloc;
		}
		for (auto& iter : scopeBlock->getStatements())
		{
			generate(iter.get(), context);
		}

		//Generate destructors for objects defined in this scope.
		if (!scopeBlock->getAllControlPathsReturn())
		{
			for (std::size_t i = blockDestructorsSize; i < context->blockDestructorGenerators.size(); ++i)
			{
				context->blockDestructorGenerators[i]();
			}
		}
		int numGeneratorsToPop = (int)context->blockDestructorGenerators.size() - (int)blockDestructorsSize;
		for (int i = 0; i < numGeneratorsToPop; ++i)
		{
			context->blockDestructorGenerators.pop_back();
		}

		if (auto iter = context->scopeValues.find(blockScopeId); iter != context->scopeValues.end())
		{
			context->scopeValues.erase(iter);
		}
		context->catContext->removeScope(blockScopeId);
		context->currentScope = previousScopeBlock;
	}
}


void LLVMCodeGenerator::generate(const AST::CatConstruct* constructor, LLVMCompileTimeContext* context)
{
	CatAssignableExpression* assignable = constructor->getAssignable();
	assert(assignable != nullptr 
		  && (assignable->getType().isReflectableObjectType()
			  || (assignable->getType().isPointerToReflectableObjectType() && assignable->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value)));
	llvm::Value* target = generate(static_cast<CatTypedExpression*>(assignable), context);

	TypeInfo* objectType = nullptr;
	if (assignable->getType().isReflectableObjectType())
	{
		objectType = assignable->getType().getObjectType();
	}
	else
	{
		objectType = assignable->getType().getPointeeType()->getObjectType();
	}

	llvm::Value* typeInfoConstantAsIntPtr = helper->createTypeInfoGlobalValue(context, objectType);

	if (!constructor->getIsCopyConstructor())
	{
		helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementConstructType, {target, typeInfoConstantAsIntPtr}, "_jc_placementConstructType", true);
	}
	else
	{
		CatArgumentList* arguments = constructor->getArgumentList();
		assert(arguments != nullptr && arguments->getNumArguments() == 1);
		llvm::Value* source = generate(arguments->getArgument(0), context);
		helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementCopyConstructType, {target, source, typeInfoConstantAsIntPtr}, "_jc_placementCopyConstructType", true);
	}

	//Generate a destructor if possible
	if (std::optional<std::string> variableName = assignable->getAssignableVariableName(); variableName.has_value() && constructor->getAutoDestruct())
	{
		std::string nameToDestruct = variableName.value();
		TypeInfo* scopeType = context->currentScope->getCustomType();
		CatScopeID scopeID = context->currentScope->getScopeId();
		context->blockDestructorGenerators.push_back([=]()
		{
			for (auto& iter : scopeType->getMembersByOrdinal())
			{
				if (Tools::equalsWhileIgnoringCase(iter.second->getMemberName(), nameToDestruct))
				{
					assert(iter.second->getType().compare(assignable->getType(), true, true));
					auto scopeIter = context->scopeValues.find(scopeID);
					if (scopeIter != context->scopeValues.end())
					{
						llvm::Value* localPtr = builder->CreateGEP(scopeIter->second, helper->createConstant((int)iter.first), Tools::append(iter.second->getMemberName(), "_ptr"));
						llvm::Value* typeInfoConstantAsIntPtr = helper->createTypeInfoGlobalValue(context, objectType);
						return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementDestructType, {localPtr, typeInfoConstantAsIntPtr}, "_jc_placementDestructType", true);					
					}
					assert(false);
					return (llvm::Value*)nullptr;
					
				}
			}
			assert(false);
			return (llvm::Value*)nullptr;
		});
	}
}


void LLVMCodeGenerator::generate(const AST::CatDestruct* destructor, LLVMCompileTimeContext* context)
{
	CatAssignableExpression* assignable = destructor->getAssignable();
	assert(assignable != nullptr 
		  && (assignable->getType().isReflectableObjectType()
			  || (assignable->getType().isPointerToReflectableObjectType() && assignable->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value)));
	llvm::Value* target = generate(static_cast<CatTypedExpression*>(assignable), context);

	TypeInfo* objectType = nullptr;
	if (assignable->getType().isReflectableObjectType())
	{
		objectType = assignable->getType().getObjectType();
	}
	else
	{
		objectType = assignable->getType().getPointeeType()->getObjectType();
	}
	llvm::Value* typeInfoConstantAsIntPtr = helper->createTypeInfoGlobalValue(context, objectType);

	helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementDestructType, {target, typeInfoConstantAsIntPtr}, "_jc_placementDestructType", true);

}


llvm::Value* LLVMCodeGenerator::generate(const AST::CatReturnStatement* returnStatement, LLVMCompileTimeContext* context)
{
	generate(context->currentFunctionDefinition->getEpilogBlock(), context);
	llvm::Value* returnValue = nullptr;
	if (!returnStatement->getType().isVoidType())
	{
		returnValue = generate(returnStatement->getReturnExpression(), context);
	}
	generateFunctionReturn(context->currentFunctionDefinition->getReturnTypeNode()->getType(), returnValue, context->currentFunction, context, context->currentClass != nullptr);
	return nullptr;
}


void LLVMCodeGenerator::generate(const AST::CatVariableDeclaration* variableDeclaration, LLVMCompileTimeContext* context)
{
	generate(variableDeclaration->getInitializationExpression(), context);
}


void LLVMCodeGenerator::generate(const AST::CatIfStatement* ifStatement, LLVMCompileTimeContext* context)
{
	llvm::Value* conditionValue = builder->CreateTrunc(generate(ifStatement->getConditionExpression(), context), targetConfig->getLLVMTypes().bool1Type);
	assert(conditionValue->getType() == targetConfig->getLLVMTypes().bool1Type);
	llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(helper->getContext(), "then", context->currentFunction);
	llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(helper->getContext(), "else");
	bool allIfControlPathsReturn = ifStatement->getAllControlPathsReturn();
	llvm::BasicBlock* continueBlock = nullptr;
	if (!allIfControlPathsReturn)
	{
		continueBlock = llvm::BasicBlock::Create(helper->getContext(), "continue");
	}
	builder->CreateCondBr(conditionValue, thenBlock, elseBlock);

	builder->SetInsertPoint(thenBlock);
	generate(ifStatement->getIfBody(), context);
	if (!allIfControlPathsReturn
		&& (ifStatement->getIfBody()->getNodeType() != CatASTNodeType::ScopeBlock 
		    || !static_cast<const CatScopeBlock*>(ifStatement->getIfBody())->containsReturnStatement()))
	{
		//Insert branch to continue block
		builder->CreateBr(continueBlock);
	}
	context->currentFunction->getBasicBlockList().push_back(elseBlock);
	builder->SetInsertPoint(elseBlock);
	if (ifStatement->getElseBody() != nullptr)
	{
		generate(ifStatement->getElseBody(), context);
	}
	if (!allIfControlPathsReturn
		&& (ifStatement->getElseBody() == nullptr 
			|| ifStatement->getElseBody()->getNodeType() != CatASTNodeType::ScopeBlock
			|| !static_cast<const CatScopeBlock*>(ifStatement->getElseBody())->containsReturnStatement()))
	{
		//Insert branch to continue block
		builder->CreateBr(continueBlock);
	}
	if (!allIfControlPathsReturn)
	{
		context->currentFunction->getBasicBlockList().push_back(continueBlock);
		builder->SetInsertPoint(continueBlock);
	}
}


void LLVMCodeGenerator::generate(const AST::CatForLoop* forLoop, LLVMCompileTimeContext* context)
{
	CatScopeID iteratorScopeId = context->catContext->addDynamicScope(forLoop->getCustomType(), nullptr);
	assert(iteratorScopeId == forLoop->getScopeId());
	llvm::Value* iteratorAlloc = helper->createObjectAllocA(context, "iterator_locals", CatGenericType(forLoop->getCustomType(), true, false), false);
	context->scopeValues[iteratorScopeId] = iteratorAlloc;
	const CatRange* range = forLoop->getRange();
	assert(range->getRangeMin()->getType().isIntType());
	assert(range->getRangeMax()->getType().isIntType());
	assert(range->getRangeStep()->getType().isIntType());
	TypeMemberInfo* memberInfo = forLoop->getCustomType()->getMembers().begin()->second.get();
	memberInfo->generateAssignCode(iteratorAlloc, generate(range->getRangeMin(), context), context);
	llvm::BasicBlock* preheaderBlock = builder->GetInsertBlock();

	//Create the basic blocks for the for-loop
	llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(helper->getContext(), "loopCondition", context->currentFunction);
	llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(helper->getContext(), "loopBody");
	bool allForControlPathsReturn = forLoop->getAllControlPathsReturn();
	llvm::BasicBlock* continueBlock = continueBlock = llvm::BasicBlock::Create(helper->getContext(), "continue");

	//Insert an explicit fall through from the current block to the conditionBlock.
	builder->CreateBr(conditionBlock);
	//Start insertion in conditionBlock.
	builder->SetInsertPoint(conditionBlock);

	// Check the loop condition
	llvm::Value* condition = builder->CreateICmpSLT(memberInfo->generateDereferenceCode(iteratorAlloc, context), generate(range->getRangeMax(), context));
	
	//Jump to either the loop block, or the continuation block.
	builder->CreateCondBr(condition, loopBlock, continueBlock);


	context->currentFunction->getBasicBlockList().push_back(loopBlock);

	// Start insertion in loopBlock.
	builder->SetInsertPoint(loopBlock);
	
	//Generate loop body
	generate(forLoop->getBody(), context);
	if (!forLoop->getBody()->getAllControlPathsReturn())
	{
		//Generate step value
		llvm::Value* stepValue = generate(range->getRangeStep(), context);

		//Increment iterator
		memberInfo->generateAssignCode(iteratorAlloc, builder->CreateAdd(memberInfo->generateDereferenceCode(iteratorAlloc, context), stepValue, "loopStep"), context);
	
		//Jump back to the condition
		builder->CreateBr(conditionBlock);
	}
	context->currentFunction->getBasicBlockList().push_back(continueBlock);
	//Continue insertion in continueBlock.
	builder->SetInsertPoint(continueBlock);

	context->scopeValues.erase(iteratorScopeId);
	context->catContext->removeScope(iteratorScopeId);
}


void LLVMCodeGenerator::generate(const AST::CatStatement* statement, LLVMCompileTimeContext* context)
{
	if (statement->isTypedExpression())
	{
		generate(static_cast<const CatTypedExpression*>(statement), context);
	}
	else
	{
		switch (statement->getNodeType())
		{
			case CatASTNodeType::ForLoop:				generate(static_cast<const CatForLoop*>(statement), context);				return;
			case CatASTNodeType::IfStatement:			generate(static_cast<const CatIfStatement*>(statement), context);			return;
			case CatASTNodeType::VariableDeclaration:	generate(static_cast<const CatVariableDeclaration*>(statement), context);	return;
			case CatASTNodeType::ScopeBlock:			generate(static_cast<const CatScopeBlock*>(statement), context);			return;
			case CatASTNodeType::Contruct:				generate(static_cast<const CatConstruct*>(statement), context);				return;
			case CatASTNodeType::Destruct:				generate(static_cast<const CatDestruct*>(statement), context);				return;
			default:									assert(false);																return;
		}
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
	ScopeCheck(context);
	assert(context->currentLib != nullptr);
	const CatClassDefinition* previousClass = context->currentClass;
	context->currentClass = classDefinition;
	CatRuntimeContext* previousContext = context->catContext;
	context->catContext = classDefinition->getCompiletimeContext();
	ErrorContext errorContext(context->catContext, classDefinition->getClassName());
	classDefinition->getCustomType()->setDylib(dylib);
	for (auto& iter: classDefinition->getClassDefinitions())
	{
		ScopeCheck(context);
		generate(iter, context);
	}
	CatScopeID classScopeId = context->catContext->addDynamicScope(classDefinition->getCustomType(), nullptr);
	assert(classScopeId == classDefinition->getScopeId());
	for (auto& iter: classDefinition->getFunctionDefinitions())
	{
		ScopeCheck(context);
		generate(iter, context);
	}
	CatFunctionDefinition* constructorDefinition = classDefinition->getFunctionDefinitionByName("__init");
	if (constructorDefinition != nullptr)
	{
		ScopeCheck(context);
		generate(constructorDefinition, context);
	}
	CatFunctionDefinition* destructorDefinition = classDefinition->getFunctionDefinitionByName("__destroy");
	if (destructorDefinition != nullptr)
	{
		ScopeCheck(context);
		generate(destructorDefinition, context);
	}
	context->currentClass = previousClass;
	context->catContext->removeScope(classScopeId);
	context->catContext = previousContext;
}


llvm::Function* LLVMCodeGenerator::generate(const AST::CatFunctionDefinition* functionDefinition, LLVMCompileTimeContext* context)
{
	ScopeCheck(context);
	assert(context->currentLib != nullptr);
	const std::string& name = functionDefinition->getMangledFunctionName(targetConfig->sretBeforeThis);
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

	llvm::FunctionType* functionType = createFunctionType(isThisCall, returnType, parameterTypes);

	llvm::Function* function = currentModule->getFunction(name);
	if (function == nullptr)
	{
		//The function was not previously defined, create a new function prototype.
		function = generateFunctionPrototype(name, functionType, isThisCall, returnType, parameterNames);
	}
	else
	{
		//Check that the previously defined function prototype has the same type.
		assert(function->getFunctionType() == functionType);
		//Check that the function has no code associated with it.
		assert(function->empty());
	}
	
	
	if (!targetConfig->callerDestroysTemporaryArguments)
	{
		int parameterOffset = 0;
		if (isThisCall)								parameterOffset++;
		if (returnType.isReflectableObjectType())	parameterOffset++;
		for (int i = 0; i < functionDefinition->getNumParameters(); i++)
		{
			if (functionDefinition->getParameterType(i).isReflectableObjectType())
			{
				llvm::Argument* argument = function->arg_begin() + (i + parameterOffset);
				context->blockDestructorGenerators.push_back([=]()
						{
							llvm::Value* typeInfoConstantAsIntPtr = helper->createTypeInfoGlobalValue(context, functionDefinition->getParameterType(i).getObjectType());
							return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementDestructType, {argument, typeInfoConstantAsIntPtr}, "_jc_placementDestructType", true);					
						});
			}
		}
	}

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
			if (targetConfig->sretBeforeThis)
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
		parametersScopeId = context->catContext->addDynamicScope(functionDefinition->getParametersType(), nullptr);
		context->scopeValues[parametersScopeId] = nullptr;
	}
	assert(parametersScopeId == functionDefinition->getScopeId());
	generate(scopeBlock, context);
	if (!functionDefinition->getAllControlPathsReturn() && returnType.isVoidType())
	{
		generateFunctionReturn(returnType, nullptr, function, context, isThisCall);
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
			std::vector<int> argumentsToCheckForNull;
			const std::vector<CatGenericType>& argumentTypes = functionInfo->getArgumentTypes();
			for (int i = 0; i < (int)argumentTypes.size(); ++i)
			{
				if (argumentTypes[i].isNonNullPointerType())
				{
					argumentsToCheckForNull.push_back(i);
				}
			}
			LLVMPreGeneratedExpression preGenerated(rValue, expression->getType().removeIndirection().toPointer());
			return helper->generateMemberFunctionCall(functionInfo, expression, {&preGenerated}, argumentsToCheckForNull, context);
		}
		assert(false);
	}
	else
	{
		switch (expression->getNodeType())
		{
			case CatASTNodeType::StaticIdentifier:		return generateAssign(static_cast<const CatStaticIdentifier*>(expression), rValue, context);
			case CatASTNodeType::MemberAccess:			return generateAssign(static_cast<const CatMemberAccess*>(expression), rValue, context);
			case CatASTNodeType::MemberFunctionCall:	return generateAssign(static_cast<const CatMemberFunctionCall*>(expression), rValue, context);
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


llvm::Value* LLVMCodeGenerator::generateAssign(const AST::CatStaticMemberAccess* memberAccess, llvm::Value* rValue, LLVMCompileTimeContext* context)
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


llvm::Value* jitcat::LLVM::LLVMCodeGenerator::generateAssign(const AST::CatMemberFunctionCall* memberFunction, llvm::Value* rValue, LLVMCompileTimeContext* context)
{
	llvm::Value* base = generate(memberFunction, context);
	helper->writeToPointer(base, rValue);
	return rValue;
}


llvm::Value* LLVMCodeGenerator::generateFPMath(const char* floatName, float(*floatVariant)(float), const char* doubleName, double(*doubleVariant)(double), 
											   const AST::CatArgumentList* argumentList, LLVMCompileTimeContext* context)
{
	if (argumentList->getArgumentType(0).isDoubleType())
	{
		return helper->createIntrinsicCall(context, doubleVariant, { generate(argumentList->getArgument(0), context)}, doubleName, true);
	}
	else
	{
		return helper->createIntrinsicCall(context, floatVariant, { helper->convertType(generate(argumentList->getArgument(0), context), argumentList->getArgumentType(0), CatGenericType::floatType, context) }, floatName, true);
	}
}


llvm::Value* LLVMCodeGenerator::generateFPMath(const char* floatName, float(*floatVariant)(float, float), const char* doubleName, double(*doubleVariant)(double, double), 
															 const AST::CatArgumentList* argumentList, LLVMCompileTimeContext* context)
{
	if (argumentList->getArgumentType(0).isDoubleType() || argumentList->getArgumentType(1).isDoubleType())
	{
		llvm::Value* x = helper->convertType(generate(argumentList->getArgument(0), context), argumentList->getArgument(0)->getType(), CatGenericType::doubleType, context);
		llvm::Value* y = helper->convertType(generate(argumentList->getArgument(1), context), argumentList->getArgument(1)->getType(), CatGenericType::doubleType, context);
		return helper->createIntrinsicCall(context, doubleVariant, { x, y }, doubleName, true);
	}
	else
	{
		llvm::Value* x = helper->convertType(generate(argumentList->getArgument(0), context), argumentList->getArgument(0)->getType(), CatGenericType::floatType, context);
		llvm::Value* y = helper->convertType(generate(argumentList->getArgument(1), context), argumentList->getArgument(1)->getType(), CatGenericType::floatType, context);
		return helper->createIntrinsicCall(context, floatVariant, { x, y }, floatName, true);
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
		if (context->isPrecompilationContext)
		{
			//If this is a precompilation code generation pass, we need to define a global variable for the global scope.
			//This global variable must then be set to the global scope before any precompiled expressions are executed.
			const std::string_view scopeName = context->catContext->getScopeNameView(scopeId);
			std::string scopeNameStr(scopeName.data(), scopeName.size());
			llvm::LoadInst* loadedGlobal = builder->CreateLoad(std::static_pointer_cast<LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalVariable(scopeNameStr, context), Tools::append(scopeNameStr, "_Ptr"));
			llvm::MDNode *MD = llvm::MDNode::get(LLVMJit::get().getContext(), llvm::None);
			loadedGlobal->setMetadata(llvm::LLVMContext::MD_nonnull, MD);
			parentObjectAddress = loadedGlobal;
			
		}
		else
		{
			unsigned char* object = context->catContext->getScopeObject(scopeId);
			parentObjectAddress = llvm::ConstantInt::get(LLVMJit::get().getContext(), llvm::APInt(sizeof(std::uintptr_t) * 8, (uint64_t)reinterpret_cast<std::uintptr_t>(object), false));
		}
	}
	else
	{
		//Get the CatRuntimeContext argument from the current function
		assert(context->currentFunction != nullptr);
		assert(context->currentFunction->arg_size() > 0);
		llvm::Argument* argument = context->currentFunction->arg_begin();
		for (std::size_t i = 0; i < context->currentFunction->arg_size(); ++i)
		{
			//i + 1 here because the first attribute applies to the return value
			if (context->currentFunction->hasAttribute((int)i + 1, llvm::Attribute::StructRet)
				|| context->currentFunction->getArg((int)i)->getName() == "__this")
			{
				continue;
			}
			argument = context->currentFunction->arg_begin() + i;
			break;
		}
		assert(argument->getName() == "RuntimeContext");
		assert(argument->getType() == targetConfig->getLLVMTypes().pointerType);
		llvm::Value* scopeIdValue = context->helper->createConstant((int)scopeId);
		llvm::Value* address = address = helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_getScopePointerFromContext, {argument, scopeIdValue}, "_jc_getScopePointerFromContext", true); 
	
		assert(address != nullptr);
		parentObjectAddress = helper->convertToIntPtr(address, "CustomThis_IntPtr");
	}

	return parentObjectAddress;
}


void LLVMCodeGenerator::initContext(LLVMCompileTimeContext* context)
{
	context->helper = helper.get();
	context->targetConfig = targetConfig;
}


void LLVMCodeGenerator::createNewModule(LLVMCompileTimeContext* context)
{
	currentModule.reset(new llvm::Module(context->catContext->getContextName(), LLVMJit::get().getContext()));
	currentModule->setTargetTriple(targetConfig->getTargetMachine().getTargetTriple().str());
	currentModule->setDataLayout(targetConfig->getDataLayout());

	// Create a new pass manager attached to it.
	passManager = std::make_unique<llvm::legacy::FunctionPassManager>(currentModule.get());

	createOptimisationPasses(passManager.get());
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


uint64_t LLVMCodeGenerator::getSymbolAddress(const std::string& name, llvm::orc::JITDylib& dyLib) const
{
	return llvm::cantFail(executionSession->lookup({&dyLib}, mangler->operator()(name))).getAddress();
}


llvm::FunctionType* LLVMCodeGenerator::createFunctionType(bool isThisCall, const CatGenericType& returnType, const std::vector<CatGenericType>& parameterTypes)
{
	//Define the parameters for the function.
	//If the function returns an object by value, the object is returned through an object pointer parameter and the function returns void.
	//The object pointer should point to pre-allocated memory where the returned object will be constructed.
	std::vector<llvm::Type*> parameters;
	llvm::FunctionType* functionType = nullptr;

	llvm::Type* functionReturnType = nullptr;

	if (isThisCall)
	{
		parameters.push_back(targetConfig->getLLVMTypes().pointerType);
	}
	if (returnType.isReflectableObjectType() || returnType.isVector4fType())
	{
		parameters.push_back(targetConfig->getLLVMTypes().pointerType);
		functionReturnType = targetConfig->getLLVMTypes().voidType;
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

	return functionType;
}


llvm::Function* LLVMCodeGenerator::generateFunctionPrototype(const std::string& functionName, bool isThisCall, const CatGenericType& returnType, const std::vector<CatGenericType>& parameterTypes, const std::vector<std::string>& parameterNames)
{
	assert(parameterTypes.size() == parameterNames.size());

	llvm::FunctionType* functionType = createFunctionType(isThisCall, returnType, parameterTypes);

	return generateFunctionPrototype(functionName, functionType, isThisCall, returnType, parameterNames);
}


llvm::Function* LLVMCodeGenerator::generateFunctionPrototype(const std::string& functionName, llvm::FunctionType* functionType, bool isThisCall, const CatGenericType& returnType, const std::vector<std::string>& parameterNames)
{
	//Create the function signature. No code is yet associated with the function at this time.
	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, functionName.c_str(), currentModule.get());
	if (isThisCall && targetConfig->useThisCall)
	{
		function->setCallingConv(llvm::CallingConv::X86_ThisCall);
	}
	else
	{
		function->setCallingConv(targetConfig->getOptions().defaultLLVMCallingConvention);
	}
	//Attributes and names for the parameters can now be set on the function signature.
	//When returning a string, the StructRet attribute is set to indicate that the parameter is used for returning a structure by value.
	llvm::Argument* currentArg = function->arg_begin();
	if (returnType.isReflectableObjectType())
	{
		if (!isThisCall || targetConfig->sretBeforeThis)
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
		if (isThisCall && targetConfig->sretBeforeThis)
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
	if (targetConfig->getOptions().explicitEnableTailCalls)		function->addFnAttr("disable-tail-calls", "false");
	if (targetConfig->getOptions().nonLeafFramePointer)			function->addFnAttr("frame-pointer", "non-leaf");
	if (targetConfig->getOptions().strongStackProtection)		
	{
		function->addFnAttr(llvm::Attribute::StackProtectStrong);
		function->addFnAttr("stack-protector-buffer-size", "8");
	}
	return function;
}


void LLVMCodeGenerator::generateFunctionReturn(const CatGenericType& returnType, llvm::Value* expressionValue, llvm::Function* function, 
											   LLVMCompileTimeContext* context, bool hasThisArgument)
{
	//If the expression returns a string or a reflectable object, copy construct it into the StructRet parameter and return void.
	//If it is some other type, just return the value.
	if (returnType.isReflectableObjectType())
	{
		llvm::Value* typeInfoConstantAsIntPtr = helper->createTypeInfoGlobalValue(context, returnType.getObjectType());
		assert(returnType.isCopyConstructible());
		llvm::Value* castPointer = builder->CreatePointerCast(expressionValue, targetConfig->getLLVMTypes().pointerType, Tools::append(returnType.toString(), "_ObjectPointerCast"));

		llvm::Argument* sretArgument = function->arg_begin();
		if (hasThisArgument && !targetConfig->sretBeforeThis)
		{
			++sretArgument;
		}
		helper->createOptionalNullCheckSelect(castPointer, 
			[&](LLVMCompileTimeContext* context)
			{
				return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementCopyConstructType, {sretArgument, castPointer, typeInfoConstantAsIntPtr}, "_jc_placementCopyConstructType", true);
			},
			[&](LLVMCompileTimeContext* context)
			{
				return helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementConstructType, {sretArgument, typeInfoConstantAsIntPtr}, "_jc_placementConstructType", true);
			}, context);

		helper->generateBlockDestructors(context);
		builder->CreateRetVoid();
	}
	else if (returnType.isVector4fType())
	{
		llvm::Argument* sretArgument = function->arg_begin();
		if (hasThisArgument && !targetConfig->sretBeforeThis)
		{
			++sretArgument;
		}
		llvm::Value* float4Ptr = builder->CreatePointerCast(sretArgument, expressionValue->getType()->getPointerTo());
		builder->CreateStore(expressionValue, float4Ptr);
		helper->generateBlockDestructors(context);
		builder->CreateRetVoid();
	}
	else if (returnType.isVoidType())
	{
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
	customType->setDylib(dylib);
	for (auto& iter : customType->getTypes())
	{
		link(static_cast<CustomTypeInfo*>(iter.second));
	}
	for (auto& iter : customType->getMemberFunctions())
	{
		if (!iter.second->isDeferredFunctionCall())
		{
			const std::string& mangledName = static_cast<CustomTypeMemberFunctionInfo*>(iter.second.get())->getFunctionDefinition()->getMangledFunctionName(targetConfig->sretBeforeThis);
			static_cast<CustomTypeMemberFunctionInfo*>(iter.second.get())->setFunctionNativeAddress((intptr_t)getSymbolAddress(mangledName, *dylib));
		}
	}
}


llvm::Module* LLVMCodeGenerator::getCurrentModule() const
{
	return currentModule.get();
}


llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* LLVMCodeGenerator::getBuilder() const
{
	return builder.get();
}


llvm::Value* LLVMCodeGenerator::booleanCast(llvm::Value* boolean)
{
	return builder->CreateCast(llvm::Instruction::CastOps::ZExt ,boolean, targetConfig->getLLVMTypes().boolType);
}


void LLVMCodeGenerator::createOptimisationPasses(llvm::legacy::FunctionPassManager* passManager)
{
	// Do simple "peephole" and bit-twiddling optimizations.
	passManager->add(llvm::createInstructionCombiningPass());
	// Reassociate expressions.
	passManager->add(llvm::createReassociatePass());
	// Eliminate Common SubExpressions.
	passManager->add(llvm::createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	passManager->add(llvm::createCFGSimplificationPass());
	// Move some alloca's to registers
	passManager->add(llvm::createPromoteMemoryToRegisterPass());

	//Initialize the passManager
	passManager->doInitialization();
}


std::unique_ptr<LLVMMemoryManager> LLVMCodeGenerator::memoryManager = std::make_unique<LLVMMemoryManager>();
