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
#include "jitcat/Tools.h"


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::LLVM;


LLVMPrecompilationContext::LLVMPrecompilationContext(LLVMTargetConfig* targetConfig, const std::string& outputFileNameWithoutExtension)
{
	addTarget(targetConfig, outputFileNameWithoutExtension);
	currentTarget = precompilationTargets[0].get();
}


LLVMPrecompilationContext::~LLVMPrecompilationContext()
{
}

void LLVMPrecompilationContext::addTarget(LLVMTargetConfig* targetConfig, const std::string& outputFileNameWithoutExtensions)
{
	precompilationTargets.emplace_back(std::make_unique<PrecompilationTarget>(outputFileNameWithoutExtensions, targetConfig));

}


void LLVMPrecompilationContext::finishPrecompilation()
{
	for (auto& iter : precompilationTargets)
	{
		iter->codeGenerator->generateExpressionSymbolEnumerationFunction(iter->compiledExpressionFunctions);
		iter->codeGenerator->generateGlobalVariablesEnumerationFunction(iter->globalVariables);
		iter->codeGenerator->generateLinkedFunctionsEnumerationFunction(iter->globalFunctionPointers);
		iter->codeGenerator->generateStringPoolInitializationFunction(iter->stringPool);
		iter->codeGenerator->emitModuleToObjectFile(Tools::append(iter->outputFileNameWithoutExtension, ".", iter->targetConfig->objectFileExtension));
	}
}


void LLVMPrecompilationContext::precompileSourceFile(const jitcat::AST::CatSourceFile* sourceFile, jitcat::CatLib* catLib, CatRuntimeContext* context)
{
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	for (auto& iter : precompilationTargets)
	{
		CatLib* previousLib = currentTarget->compileContext->currentLib;

		currentTarget = iter.get();
		currentTarget->compileContext->clearState();
		currentTarget->compileContext->catContext = context;
		context->setCodeGenerator(currentTarget->codeGenerator);
		currentTarget->compileContext->currentLib = catLib;
		currentTarget->codeGenerator->generate(sourceFile, currentTarget->compileContext.get());

		currentTarget->compileContext->currentLib = previousLib;
	}
	context->setCodeGenerator(oldGenerator);
}


void LLVMPrecompilationContext::precompileExpression(const CatTypedExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, CatRuntimeContext* context)
{
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	for (auto& iter : precompilationTargets)
	{
		currentTarget = iter.get();
		currentTarget->compileContext->clearState();
		currentTarget->compileContext->catContext = context;
		context->setCodeGenerator(currentTarget->codeGenerator);
		const std::string expressionName = ExpressionHelperFunctions::getUniqueExpressionFunctionName(expressionStr, currentTarget->compileContext->catContext, false, expectedType);
		if (currentTarget->compiledExpressionFunctions.find(expressionName) == currentTarget->compiledExpressionFunctions.end())
		{
			llvm::Function* function = currentTarget->codeGenerator->generateExpressionFunction(expression, currentTarget->compileContext.get(), expressionName);
			currentTarget->compiledExpressionFunctions.insert(std::make_pair(expressionName, function));
		}
	}
	context->setCodeGenerator(oldGenerator);
}


void LLVMPrecompilationContext::precompileAssignmentExpression(const CatAssignableExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, CatRuntimeContext* context)
{
	std::shared_ptr<LLVMCodeGenerator> oldGenerator = context->getCodeGenerator();
	for (auto& iter : precompilationTargets)
	{
		currentTarget = iter.get();
		currentTarget->compileContext->clearState();
		currentTarget->compileContext->catContext = context;
		context->setCodeGenerator(currentTarget->codeGenerator);
		const std::string expressionName = ExpressionHelperFunctions::getUniqueExpressionFunctionName(expressionStr, iter->compileContext->catContext, true, expectedType);
		if (iter->compiledExpressionFunctions.find(expressionName) == iter->compiledExpressionFunctions.end())
		{
			llvm::Function* function = iter->codeGenerator->generateExpressionAssignFunction(expression, iter->compileContext.get(), expressionName);
			iter->compiledExpressionFunctions.insert(std::make_pair(expressionName, function));
		}
	}
	context->setCodeGenerator(oldGenerator);
}


llvm::GlobalVariable* LLVMPrecompilationContext::defineGlobalVariable(const std::string& globalSymbolName, LLVMCompileTimeContext* context)
{
	auto iter = currentTarget->globalVariables.find(globalSymbolName);
	if (iter != currentTarget->globalVariables.end())
	{
		return iter->second;
	}
	else
	{
		llvm::GlobalVariable* global = context->helper->createGlobalPointerSymbol(globalSymbolName);
		currentTarget->globalVariables.insert(std::make_pair(globalSymbolName, global));
		return global;
	}
}


llvm::GlobalVariable* LLVMPrecompilationContext::defineGlobalFunctionPointer(const std::string& globalSymbolName, LLVMCompileTimeContext* context)
{
	auto iter = currentTarget->globalFunctionPointers.find(globalSymbolName);
	if (iter != currentTarget->globalFunctionPointers.end())
	{
		return iter->second;
	}
	else
	{
		llvm::GlobalVariable* global = context->helper->createGlobalPointerSymbol(globalSymbolName);
		currentTarget->globalFunctionPointers.insert(std::make_pair(globalSymbolName, global));
		return global;
	}
}


llvm::GlobalVariable* LLVMPrecompilationContext::defineGlobalString(const std::string& stringValue, LLVMCompileTimeContext* context)
{
	auto iter = currentTarget->stringPool.find(stringValue);
	if (iter != currentTarget->stringPool.end())
	{
		return iter->second;
	}
	else
	{
		llvm::GlobalVariable* global = context->helper->createGlobalPointerSymbol(Tools::append(stringValue, ".str"));
		currentTarget->stringPool.insert(std::make_pair(stringValue, global));
		return global;
	}
}


LLVMPrecompilationContext::PrecompilationTarget::PrecompilationTarget(const std::string& outputFileNameWithoutExtension, LLVMTargetConfig* targetConfig):
	outputFileNameWithoutExtension(outputFileNameWithoutExtension),
	targetConfig(targetConfig),
	codeGenerator(std::make_shared<LLVMCodeGenerator>(outputFileNameWithoutExtension, targetConfig)),
	compileContext(std::make_unique<LLVMCompileTimeContext>(nullptr, targetConfig, true))
{
	compileContext->options.enableDereferenceNullChecks = true;
}


LLVMPrecompilationContext::PrecompilationTarget::~PrecompilationTarget()
{
}
