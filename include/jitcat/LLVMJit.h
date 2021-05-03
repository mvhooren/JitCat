/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/ObjectCache.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/Value.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SmallVectorMemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Scalar.h>

#include <iostream>
#include <map>
#include <memory>

namespace jitcat
{
	class PrecompilationContext;
};


namespace jitcat::LLVM
{
	class LLVMCodeGenerator;
	class LLVMJit;
	class LLVMPrecompilationContext;
	class LLVMTargetConfig;

	class LLVMJitInitializer
	{
		friend class LLVMJit;
		LLVMJitInitializer();
	};

	class LLVMJit: private LLVMJitInitializer
	{
	private:
		LLVMJit();
		~LLVMJit();
		LLVMJit(const LLVMJit&) = delete;
		LLVMJit& operator= (const LLVMJit&) = delete;

	public:
		static LLVMJit& get();

		template <typename... Arguments>
		static llvm::Value* logError(Arguments ... arguments);
	
		llvm::LLVMContext& getContext() const;
		llvm::orc::ThreadSafeContext& getThreadSafeContext() const;
		std::shared_ptr<llvm::orc::SymbolStringPool> getSymbolStringPool() const;

		const LLVMTargetConfig* getJitTargetConfig() const;

		void cleanup();

		std::shared_ptr<PrecompilationContext> createLLVMPrecompilationContext();
	private:
		void initJitTarget();

	private:
		static LLVMJit* instance;
		//A thread-safe version of a LLVM Context. 
		//LLVM functionality is isolated per context. For instance, modules and types created on different contexts cannot interact.
		//Used for building LLVM IR modules.
		std::unique_ptr<llvm::orc::ThreadSafeContext> context;

		//Target configuration for JIT code generation.
		std::unique_ptr<LLVMTargetConfig> jitTargetConfig;

		//A string pool that is shared among executionSessions. It stores symbol names.
		std::shared_ptr<llvm::orc::SymbolStringPool> symbolStringPool;
	};


	#include "jitcat/LLVMJitHeaderImplementation.h"
} //End namespace jitcat::LLVM
