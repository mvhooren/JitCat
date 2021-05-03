/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMPrecompilationContext.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/ExpressionHelperFunctions.h"
#include "jitcat/LLVMCodeGenerator.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMJit.h"


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::LLVM;


LLVMPrecompilationContext::LLVMPrecompilationContext():
	codeGenerator(std::make_shared<LLVMCodeGenerator>("Precompilation", LLVM::LLVMJit::get().getJitTargetConfig())),
	compileContext(std::make_unique<LLVMCompileTimeContext>(nullptr, LLVM::LLVMJit::get().getJitTargetConfig(), true))
{
	compileContext->options.enableDereferenceNullChecks = true;
}


LLVMPrecompilationContext::~LLVMPrecompilationContext()
{
}


void LLVMPrecompilationContext::finishPrecompilation()
{
	codeGenerator->generateExpressionSymbolEnumerationFunction(compiledExpressionFunctions);
	codeGenerator->generateGlobalVariablesEnumerationFunction(globalVariables);
	codeGenerator->generateLinkedFunctionsEnumerationFunction(globalFunctionPointers);
	codeGenerator->generateStringPoolInitializationFunction(stringPool);
	codeGenerator->emitModuleToObjectFile("PrecompiledJitCatExpressions.obj");
}


void LLVMPrecompilationContext::precompileSourceFile(const jitcat::AST::CatSourceFile* sourceFile, jitcat::CatLib* catLib, CatRuntimeContext* context)
{
	compileContext->clearState();
	compileContext->catContext = context;
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	context->setCodeGenerator(codeGenerator);
	CatLib* previousLib = compileContext->currentLib;
	compileContext->currentLib = catLib;
	codeGenerator->generate(sourceFile, compileContext.get());
	context->setCodeGenerator(oldGenerator);
	compileContext->currentLib = previousLib;
}


void LLVMPrecompilationContext::precompileExpression(const CatTypedExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, CatRuntimeContext* context)
{
	compileContext->clearState();
	compileContext->catContext = context;
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	context->setCodeGenerator(codeGenerator);
	const std::string expressionName = ExpressionHelperFunctions::getUniqueExpressionFunctionName(expressionStr, compileContext->catContext, false, expectedType);
	if (compiledExpressionFunctions.find(expressionName) == compiledExpressionFunctions.end())
	{
		llvm::Function* function = codeGenerator->generateExpressionFunction(expression, compileContext.get(), expressionName);
		compiledExpressionFunctions.insert(std::make_pair(expressionName, function));
	}
	context->setCodeGenerator(oldGenerator);
}


void LLVMPrecompilationContext::precompileAssignmentExpression(const CatAssignableExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, CatRuntimeContext* context)
{
	compileContext->clearState();
	compileContext->catContext = context;
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	context->setCodeGenerator(codeGenerator);
	const std::string expressionName = ExpressionHelperFunctions::getUniqueExpressionFunctionName(expressionStr, compileContext->catContext, true, expectedType);
	if (compiledExpressionFunctions.find(expressionName) == compiledExpressionFunctions.end())
	{
		llvm::Function* function = codeGenerator->generateExpressionAssignFunction(expression, compileContext.get(), expressionName);
		compiledExpressionFunctions.insert(std::make_pair(expressionName, function));
	}
	context->setCodeGenerator(oldGenerator);
}


llvm::GlobalVariable* LLVMPrecompilationContext::defineGlobalVariable(const std::string& globalSymbolName, LLVMCompileTimeContext* context)
{
	auto iter = globalVariables.find(globalSymbolName);
	if (iter != globalVariables.end())
	{
		return iter->second;
	}
	else
	{
		llvm::GlobalVariable* global = context->helper->createGlobalPointerSymbol(globalSymbolName);
		globalVariables.insert(std::make_pair(globalSymbolName, global));
		return global;
	}
}


llvm::GlobalVariable* LLVMPrecompilationContext::defineGlobalFunctionPointer(const std::string& globalSymbolName, LLVMCompileTimeContext* context)
{
	auto iter = globalFunctionPointers.find(globalSymbolName);
	if (iter != globalFunctionPointers.end())
	{
		return iter->second;
	}
	else
	{
		llvm::GlobalVariable* global = context->helper->createGlobalPointerSymbol(globalSymbolName);
		globalFunctionPointers.insert(std::make_pair(globalSymbolName, global));
		return global;
	}
}


llvm::GlobalVariable* LLVMPrecompilationContext::defineGlobalString(const std::string& stringValue, LLVMCompileTimeContext* context)
{
	auto iter = stringPool.find(stringValue);
	if (iter != stringPool.end())
	{
		return iter->second;
	}
	else
	{
		llvm::GlobalVariable* global = context->helper->createGlobalPointerSymbol(Tools::append(stringValue, ".str"));
		stringPool.insert(std::make_pair(stringValue, global));
		return global;
	}
}