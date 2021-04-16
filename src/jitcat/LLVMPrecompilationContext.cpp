/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMPrecompilationContext.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
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
	codeGenerator->emitModuleToObjectFile("PrecompiledJitCatExpressions.obj");
}


void LLVMPrecompilationContext::precompileExpression(const CatTypedExpression* expression, const std::string& expressionStr, CatRuntimeContext* context)
{
	compileContext->clearState();
	compileContext->catContext = context;
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	context->setCodeGenerator(codeGenerator);
	const std::string expressionName = codeGenerator->getUniqueExpressionFunctionName(expressionStr, compileContext.get(), false);
	if (compiledFunctions.find(expressionName) == compiledFunctions.end())
	{
		compiledFunctions.insert(expressionName);
		llvm::Function* function = codeGenerator->generateExpressionFunction(expression, compileContext.get(), expressionName);
	}
	context->setCodeGenerator(oldGenerator);
}


void LLVMPrecompilationContext::precompileAssignmentExpression(const CatAssignableExpression* expression, const std::string& expressionStr, CatRuntimeContext* context)
{
	compileContext->clearState();
	compileContext->catContext = context;
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	context->setCodeGenerator(codeGenerator);
	const std::string expressionName = codeGenerator->getUniqueExpressionFunctionName(expressionStr, compileContext.get(), true);
	if (compiledFunctions.find(expressionName) == compiledFunctions.end())
	{
		compiledFunctions.insert(expressionName);
		llvm::Function* function = codeGenerator->generateExpressionAssignFunction(expression, compileContext.get(), expressionName);
	}
	context->setCodeGenerator(oldGenerator);
}
