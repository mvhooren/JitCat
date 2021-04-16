/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMJit.h"
#include "jitcat/Configuration.h"
#include "jitcat/LLVMPrecompilationContext.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/Tools.h"


#include <iostream>

using namespace jitcat;
using namespace jitcat::LLVM;


LLVMJit::LLVMJit():
	context(std::make_unique<llvm::orc::ThreadSafeContext>(std::make_unique<llvm::LLVMContext>())),
	targetMachineBuilder(llvm::cantFail(llvm::orc::JITTargetMachineBuilder::detectHost())),
	targetMachine(llvm::cantFail(targetMachineBuilder.createTargetMachine())),
	dataLayout(std::make_unique<llvm::DataLayout>(llvm::cantFail(targetMachineBuilder.getDefaultDataLayoutForTarget()))),
	symbolStringPool(std::make_shared<llvm::orc::SymbolStringPool>())
{
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
	static LLVMJit instance;
	return instance;
}


llvm::LLVMContext& LLVMJit::getContext() const
{
	return *context->getContext();
}


llvm::orc::ThreadSafeContext& jitcat::LLVM::LLVMJit::getThreadSafeContext() const
{
	return *context;
}


std::shared_ptr<llvm::orc::SymbolStringPool> jitcat::LLVM::LLVMJit::getSymbolStringPool() const
{
	return symbolStringPool;
}


llvm::TargetMachine& LLVMJit::getTargetMachine() const
{
	return *targetMachine;
}


const llvm::orc::JITTargetMachineBuilder& jitcat::LLVM::LLVMJit::getTargetMachineBuilder() const
{
	return targetMachineBuilder;
}


const llvm::DataLayout& LLVMJit::getDataLayout() const
{
	return *dataLayout;
}


void jitcat::LLVM::LLVMJit::cleanup()
{
	targetMachine.reset(nullptr);
	dataLayout.reset(nullptr);
	context.reset(nullptr);
}


std::shared_ptr<PrecompilationContext> LLVMJit::createLLVMPrecompilationContext()
{
	return std::make_shared<LLVMPrecompilationContext>();
}


LLVMJitInitializer::LLVMJitInitializer()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();
}
