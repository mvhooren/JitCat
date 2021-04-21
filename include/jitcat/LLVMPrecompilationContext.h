/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/PrecompilationContext.h"

#include <memory>
#include <string>
#include <unordered_map>


namespace jitcat::LLVM
{
	class LLVMCodeGenerator;
	struct LLVMCompileTimeContext;

	class LLVMPrecompilationContext: public PrecompilationContext
	{
	public:
		LLVMPrecompilationContext();
		~LLVMPrecompilationContext();

		// Inherited via PrecompilationContext
		virtual void finishPrecompilation() override final;

		virtual void precompileExpression(const jitcat::AST::CatTypedExpression* expression, const std::string& expressionStr, CatRuntimeContext* context) override final;
		virtual void precompileAssignmentExpression(const jitcat::AST::CatAssignableExpression* expression, const std::string& expressionStr, CatRuntimeContext* context) override final;

		llvm::GlobalVariable* defineGlobalVariable(const std::string& globalSymbolName, LLVMCompileTimeContext* context);
		llvm::GlobalVariable* defineGlobalFunctionPointer(const std::string& globalSymbolName, LLVMCompileTimeContext* context);

	private:
		std::unique_ptr<LLVMCompileTimeContext> compileContext;
		std::shared_ptr<LLVMCodeGenerator> codeGenerator;

		std::unordered_map<std::string, llvm::Function*> compiledExpressionFunctions;
		std::unordered_map<std::string, llvm::GlobalVariable*> globalVariables;
		std::unordered_map<std::string, llvm::GlobalVariable*> globalFunctionPointers;
	};
};