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


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::LLVM;


LLVMPrecompilationContext::LLVMPrecompilationContext():
	codeGenerator(std::make_shared<LLVMCodeGenerator>("Precompilation")),
	compileContext(std::make_unique<LLVMCompileTimeContext>(nullptr, true))
{
	compileContext->options.enableDereferenceNullChecks = true;
}


LLVMPrecompilationContext::~LLVMPrecompilationContext()
{
}


void LLVMPrecompilationContext::finishPrecompilation()
{
	codeGenerator->generateExpressionSymbolEnumerationFunction(compiledFunctions);
	codeGenerator->generateGlobalScopesEnumerationFunction(globalScopes);
	codeGenerator->emitModuleToObjectFile("PrecompiledJitCatExpressions.obj");
}


void LLVMPrecompilationContext::precompileExpression(const CatTypedExpression* expression, const std::string& expressionStr, CatRuntimeContext* context)
{
	compileContext->clearState();
	compileContext->catContext = context;
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	context->setCodeGenerator(codeGenerator);
	const std::string expressionName = ExpressionHelperFunctions::getUniqueExpressionFunctionName(expressionStr, compileContext->catContext, false);
	if (compiledFunctions.find(expressionName) == compiledFunctions.end())
	{
		llvm::Function* function = codeGenerator->generateExpressionFunction(expression, compileContext.get(), expressionName);
		compiledFunctions.insert(std::make_pair(expressionName, function));
	}
	context->setCodeGenerator(oldGenerator);
}


void LLVMPrecompilationContext::precompileAssignmentExpression(const CatAssignableExpression* expression, const std::string& expressionStr, CatRuntimeContext* context)
{
	compileContext->clearState();
	compileContext->catContext = context;
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	context->setCodeGenerator(codeGenerator);
	const std::string expressionName = ExpressionHelperFunctions::getUniqueExpressionFunctionName(expressionStr, compileContext->catContext, true);
	if (compiledFunctions.find(expressionName) == compiledFunctions.end())
	{
		llvm::Function* function = codeGenerator->generateExpressionAssignFunction(expression, compileContext.get(), expressionName);
		compiledFunctions.insert(std::make_pair(expressionName, function));
	}
	context->setCodeGenerator(oldGenerator);
}


llvm::GlobalVariable* jitcat::LLVM::LLVMPrecompilationContext::defineGlobalScope(const std::string& globalSymbolName, LLVMCompileTimeContext* context)
{
	auto iter = globalScopes.find(globalSymbolName);
	if (iter != globalScopes.end())
	{
		return iter->second;
	}
	else
	{
		llvm::GlobalVariable* global = context->helper->createGlobalPointerSymbol(globalSymbolName);
		globalScopes.insert(std::make_pair(globalSymbolName, global));
		return global;
	}
}
