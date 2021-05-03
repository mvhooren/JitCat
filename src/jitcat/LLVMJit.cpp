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
	jitTargetConfig = LLVMTargetConfig::createJITTargetConfig();
	LLVMTypes::doubleType = llvm::Type::getDoubleTy(*context->getContext());
	LLVMTypes::floatType = llvm::Type::getFloatTy(*context->getContext());
	LLVMTypes::intType = llvm::Type::getInt32Ty(*context->getContext());
	LLVMTypes::longintType = llvm::Type::getInt64Ty(*context->getContext());
	LLVMTypes::charType = llvm::Type::getInt8Ty(*context->getContext());
	LLVMTypes::boolType = llvm::Type::getIntNTy(*context->getContext(), sizeof(bool) * 8);
	LLVMTypes::bool1Type = llvm::Type::getInt1Ty(*context->getContext());
	LLVMTypes::pointerType = llvm::Type::getInt8PtrTy(*context->getContext());
	LLVMTypes::pointerTypeAsType = static_cast<llvm::Type*>(LLVMTypes::pointerType);
	if constexpr (sizeof(uintptr_t) == 8)
	{
		LLVMTypes::uintPtrType = llvm::Type::getInt64Ty(*context->getContext());
	}
	else
	{
		LLVMTypes::uintPtrType = llvm::Type::getInt32Ty(*context->getContext());
	}
	LLVMTypes::voidType = llvm::Type::getVoidTy(*context->getContext());

	LLVMTypes::functionRetPtrArgPtr = llvm::FunctionType::get(LLVMTypes::pointerType, {LLVMTypes::pointerType}, false);
	LLVMTypes::functionRetPtrArgPtr_Ptr = llvm::FunctionType::get(LLVMTypes::pointerType, {LLVMTypes::pointerType, LLVMTypes::pointerType}, false);
	LLVMTypes::functionRetPtrArgPtr_Int = llvm::FunctionType::get(LLVMTypes::pointerType, {LLVMTypes::pointerType, LLVMTypes::intType}, false);
	LLVMTypes::functionRetPtrArgPtr_StringPtr = llvm::FunctionType::get(LLVMTypes::pointerType, {LLVMTypes::pointerType, LLVMTypes::pointerType}, false);
}


LLVMJit::~LLVMJit()
{
}


LLVMJit& LLVMJit::get()
{
	if (instance == nullptr)
	{
		instance = new LLVMJit;
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


const LLVMTargetConfig* jitcat::LLVM::LLVMJit::getJitTargetConfig() const
{
	return jitTargetConfig.get();
}


void LLVM::LLVMJit::cleanup()
{
	context.reset(nullptr);
}


std::shared_ptr<PrecompilationContext> LLVMJit::createLLVMPrecompilationContext()
{
	return std::make_shared<LLVMPrecompilationContext>();
}


LLVMJit* LLVMJit::instance = nullptr;


LLVMJitInitializer::LLVMJitInitializer()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();
}
