/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMJit.h"
#include "jitcat/Configuration.h"
#include "jitcat/LLVMPrecompilationContext.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/LLVMTargetConfig.h"
#include "jitcat/Tools.h"


#include <iostream>

using namespace jitcat;
using namespace jitcat::LLVM;


LLVMJit::LLVMJit():
	context(std::make_unique<llvm::orc::ThreadSafeContext>(std::make_unique<llvm::LLVMContext>())),
	symbolStringPool(std::make_shared<llvm::orc::SymbolStringPool>())
{
}


LLVMJit::~LLVMJit()
{
}


LLVMJit& LLVMJit::get()
{
	if (instance == nullptr)
	{
		instance = new LLVMJit();
		//initJitTarget accesses LLVMJit to get its context.
		instance->initJitTarget();
	}
	return *instance;
}


llvm::LLVMContext& LLVMJit::getContext() const
{
	return *context->getContext();
}


llvm::orc::ThreadSafeContext& LLVM::LLVMJit::getThreadSafeContext() const
{
	return *context;
}


std::shared_ptr<llvm::orc::SymbolStringPool> LLVM::LLVMJit::getSymbolStringPool() const
{
	return symbolStringPool;
}


const LLVMTargetConfig* LLVMJit::getJitTargetConfig() const
{
	return jitTargetConfig.get();
}


void LLVM::LLVMJit::cleanup()
{
	context.reset(nullptr);
}


void LLVMJit::initJitTarget()
{
	jitTargetConfig = LLVMTargetConfig::createJITTargetConfig();
}


LLVMJit* LLVMJit::instance = nullptr;


LLVMJitInitializer::LLVMJitInitializer()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();
}
