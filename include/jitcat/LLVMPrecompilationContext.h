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
#include <vector>


namespace jitcat::LLVM
{
	class LLVMCodeGenerator;
	struct LLVMCompileTimeContext;
	class LLVMTargetConfig;

	class LLVMPrecompilationContext: public PrecompilationContext
	{
		struct PrecompilationTarget
		{
			PrecompilationTarget(const std::string& outputFileNameWithoutExtension, LLVMTargetConfig* targetConfig);
			~PrecompilationTarget();
			std::string outputFileNameWithoutExtension;
			LLVMTargetConfig* targetConfig;
			std::unique_ptr<LLVMCompileTimeContext> compileContext;
			std::shared_ptr<LLVMCodeGenerator> codeGenerator;

			std::unordered_map<std::string, llvm::Function*> compiledExpressionFunctions;
			std::unordered_map<std::string, llvm::GlobalVariable*> globalVariables;
			std::unordered_map<std::string, llvm::GlobalVariable*> globalFunctionPointers;
			std::unordered_map<std::string, llvm::GlobalVariable*> stringPool;
		};
	public:
		LLVMPrecompilationContext(LLVMTargetConfig* targetConfig, const std::string& outputFileNameWithoutExtension);
		~LLVMPrecompilationContext();

		void addTarget(LLVMTargetConfig* targetConfig, const std::string& outputFileNameWithoutExtension);

		// Inherited via PrecompilationContext
		virtual void finishPrecompilation() override final;

		virtual void precompileSourceFile(const jitcat::AST::CatSourceFile* sourceFile,  jitcat::CatLib* catLib, CatRuntimeContext* context) override final;
		virtual void precompileExpression(const jitcat::AST::CatTypedExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, CatRuntimeContext* context) override final;
		virtual void precompileAssignmentExpression(const jitcat::AST::CatAssignableExpression* expression, const std::string& expressionStr, const CatGenericType& expectedType, CatRuntimeContext* context) override final;

		llvm::GlobalVariable* defineGlobalVariable(const std::string& globalSymbolName, LLVMCompileTimeContext* context);
		llvm::GlobalVariable* defineGlobalFunctionPointer(const std::string& globalSymbolName, LLVMCompileTimeContext* context);
		llvm::GlobalVariable* defineGlobalString(const std::string& stringValue, LLVMCompileTimeContext* context);

	private:
		PrecompilationTarget* currentTarget;
		std::vector<std::unique_ptr<PrecompilationTarget>> precompilationTargets;
	};
};