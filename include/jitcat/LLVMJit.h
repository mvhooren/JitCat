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

namespace jitcat::LLVM
{
	class LLVMCodeGenerator;
	class LLVMJit;


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

		llvm::TargetMachine& getTargetMachine() const;
		const llvm::DataLayout& getDataLayout() const;

		llvm::orc::JITDylib& createDyLib(const std::string& name);

		void addModule(std::unique_ptr<llvm::Module>& module, llvm::orc::JITDylib& dyLib);

		llvm::Expected<llvm::JITEvaluatedSymbol> findSymbol(const std::string& name, llvm::orc::JITDylib& dyLib) const;
		llvm::JITTargetAddress getSymbolAddress(const std::string& name, llvm::orc::JITDylib& dyLib) const;

	private:
		//A thread-safe version of a LLVM Context. 
		//LLVM functionality is isolated per context. For instance, modules and types created on different contexts cannot interact.
		//Used for building LLVM IR modules.
		std::unique_ptr<llvm::orc::ThreadSafeContext> context;
		//A helper class for building the target machine information.
		llvm::orc::JITTargetMachineBuilder targetMachineBuilder;
		//ExecutionSession represents a running JIT program
		std::unique_ptr<llvm::orc::ExecutionSession> executionSession;
		//Contains all the target specific information for the machine that we are compiling for. Among other things, the target CPU type.
		std::unique_ptr<llvm::TargetMachine> targetMachine;
		//Specifies the layout of structs and the type of name mangling used based on the target machine as well as endianness.
		std::unique_ptr<const llvm::DataLayout> dataLayout;
		//std::shared_ptr<llvm::orc::SymbolResolver> symbolResolver;
		//Mangles symbol names
		std::unique_ptr<llvm::orc::MangleAndInterner> mangler;
		//Can be used to add object files to the JIT.
		std::unique_ptr<llvm::orc::RTDyldObjectLinkingLayer> objectLinkLayer;
		//Takes an LLVM IR module and creates an object file that is linked into the JIT using the objectLinkLayer
		std::unique_ptr<llvm::orc::IRCompileLayer> compileLayer;
		//The runtime library dylib
		llvm::orc::JITDylib* runtimeLibraryDyLib;

		int nextDyLibIndex;
	};


	#include "jitcat/LLVMJitHeaderImplementation.h"
} //End namespace jitcat::LLVM
