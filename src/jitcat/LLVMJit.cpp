/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMJit.h"
#include "jitcat/Configuration.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/Tools.h"


#include <iostream>

using namespace jitcat;
using namespace jitcat::LLVM;


LLVMJit::LLVMJit():
	context(new llvm::orc::ThreadSafeContext(std::make_unique<llvm::LLVMContext>())),
	targetMachineBuilder(llvm::cantFail(llvm::orc::JITTargetMachineBuilder::detectHost())),
	targetMachine(std::move(llvm::cantFail(targetMachineBuilder.createTargetMachine()))),
	dataLayout(new llvm::DataLayout(llvm::cantFail(targetMachineBuilder.getDefaultDataLayoutForTarget()))),
	nextDyLibIndex(0)
{
    //executionSession->getMainJITDylib().setGenerator(llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(*dataLayout)));
	LLVMTypes::doubleType = llvm::Type::getDoubleTy(*context->getContext());
	LLVMTypes::floatType = llvm::Type::getFloatTy(*context->getContext());
	LLVMTypes::intType = llvm::Type::getInt32Ty(*context->getContext());
	LLVMTypes::charType = llvm::Type::getInt8Ty(*context->getContext());
	LLVMTypes::ucharType = llvm::Type::getInt8Ty(*context->getContext());
	LLVMTypes::boolType = llvm::Type::getInt1Ty(*context->getContext());
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


LLVMJitInitializer::LLVMJitInitializer()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();
}
