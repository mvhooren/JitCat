/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "LLVMJit.h"
#include "Configuration.h"
#include "LLVMTypes.h"
#include "Tools.h"


#include <iostream>


LLVMJit::LLVMJit():
	context(new llvm::orc::ThreadSafeContext(llvm::make_unique<llvm::LLVMContext>())),
	targetMachineBuilder(llvm::cantFail(llvm::orc::JITTargetMachineBuilder::detectHost())),
	targetMachine(std::move(llvm::cantFail(targetMachineBuilder.createTargetMachine()))),
	executionSession(new llvm::orc::ExecutionSession()),
	dataLayout(new llvm::DataLayout(llvm::cantFail(targetMachineBuilder.getDefaultDataLayoutForTarget()))),
	mangler(new llvm::orc::MangleAndInterner(*executionSession, *dataLayout)),
	objectLinkLayer(new llvm::orc::RTDyldObjectLinkingLayer(*executionSession,
															[]() {	return llvm::make_unique<llvm::SectionMemoryManager>();})),
	compileLayer(new llvm::orc::IRCompileLayer(*executionSession.get(), *(objectLinkLayer.get()), llvm::orc::ConcurrentIRCompiler(targetMachineBuilder))),
	nextDyLibIndex(0)
{
	if constexpr (Configuration::enableSymbolSearchWorkaround)
	{
		objectLinkLayer->setAutoClaimResponsibilityForObjectSymbols(true);
		objectLinkLayer->setOverrideObjectFlagsWithResponsibilityFlags(true);
	}
    //executionSession->getMainJITDylib().setGenerator(llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(*dataLayout)));
	LLVMTypes::floatType = llvm::Type::getFloatTy(*context->getContext());
	LLVMTypes::intType = llvm::Type::getInt32Ty(*context->getContext());
	LLVMTypes::boolType = llvm::Type::getInt1Ty(*context->getContext());
	LLVMTypes::pointerType = llvm::Type::getInt8PtrTy(*context->getContext());
	if constexpr (sizeof(uintptr_t) == 8)
	{
		LLVMTypes::uintPtrType = llvm::Type::getInt64Ty(*context->getContext());
	}
	else
	{
		LLVMTypes::uintPtrType = llvm::Type::getInt32Ty(*context->getContext());
	}
	LLVMTypes::voidType = llvm::Type::getVoidTy(*context->getContext());

	std::vector<llvm::Type*> structMembers = {llvm::ArrayType::get(llvm::Type::getInt8Ty(*context->getContext()), sizeof(std::string))};
	LLVMTypes::stringType = llvm::StructType::create(structMembers, "std::string");
	LLVMTypes::stringPtrType = llvm::PointerType::get(LLVMTypes::stringType, 0);

	LLVMTypes::functionRetPtrArgPtr = llvm::FunctionType::get(LLVMTypes::pointerType, {LLVMTypes::pointerType}, false);
	LLVMTypes::functionRetPtrArgPtr_Ptr = llvm::FunctionType::get(LLVMTypes::pointerType, {LLVMTypes::pointerType, LLVMTypes::pointerType}, false);
	LLVMTypes::functionRetPtrArgPtr_Int = llvm::FunctionType::get(LLVMTypes::pointerType, {LLVMTypes::pointerType, LLVMTypes::intType}, false);
	LLVMTypes::functionRetPtrArgPtr_StringPtr = llvm::FunctionType::get(LLVMTypes::pointerType, {LLVMTypes::pointerType, LLVMTypes::stringPtrType}, false);

	
	llvm::orc::SymbolMap intrinsicSymbols;
	runtimeLibraryDyLib = &executionSession->createJITDylib("runtimeLibrary", false);

	llvm::JITSymbolFlags functionFlags;
	functionFlags |= llvm::JITSymbolFlags::Callable;
	functionFlags |= llvm::JITSymbolFlags::Exported;
	functionFlags |= llvm::JITSymbolFlags::Absolute;

	intrinsicSymbols[executionSession->intern("fmodf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&fmodf), functionFlags);
	intrinsicSymbols[executionSession->intern("_fmod")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&fmodl), functionFlags);
	intrinsicSymbols[executionSession->intern("sinf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&sinf), functionFlags);
	intrinsicSymbols[executionSession->intern("_sin")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&sinl), functionFlags);
	intrinsicSymbols[executionSession->intern("cosf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&cosf), functionFlags);
	intrinsicSymbols[executionSession->intern("_cos")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&cosl), functionFlags);
	intrinsicSymbols[executionSession->intern("log10f")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&log10f), functionFlags);
	intrinsicSymbols[executionSession->intern("_log10")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&log10l), functionFlags);
	intrinsicSymbols[executionSession->intern("powf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&powf), functionFlags);
	intrinsicSymbols[executionSession->intern("_pow")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&powl), functionFlags);
	intrinsicSymbols[executionSession->intern("ceilf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&ceilf), functionFlags);
	intrinsicSymbols[executionSession->intern("_ceil")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&ceill), functionFlags);
	intrinsicSymbols[executionSession->intern("floorf")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&floorf), functionFlags);
	intrinsicSymbols[executionSession->intern("_floor")] = llvm::JITEvaluatedSymbol(reinterpret_cast<llvm::JITTargetAddress>(&floorl), functionFlags);
	

	runtimeLibraryDyLib->define(llvm::orc::absoluteSymbols(intrinsicSymbols));

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


llvm::TargetMachine& LLVMJit::getTargetMachine() const
{
	return *targetMachine;
}


const llvm::DataLayout& LLVMJit::getDataLayout() const
{
	return *dataLayout;
}


llvm::orc::JITDylib& LLVMJit::createDyLib(const std::string& name)
{
	llvm::orc::JITDylib& dylib = executionSession->createJITDylib(Tools::append(name, "_", nextDyLibIndex++), false);
	dylib.addToSearchOrder(*runtimeLibraryDyLib, false);
	return dylib;
}


void LLVMJit::addModule(std::unique_ptr<llvm::Module>& module, llvm::orc::JITDylib& dyLib)
{
    // Add the module to the JIT with a new VModuleKey.
	llvm::cantFail(compileLayer->add(dyLib, llvm::orc::ThreadSafeModule(std::move(module), *context.get())));
}


llvm::Expected<llvm::JITEvaluatedSymbol> LLVMJit::findSymbol(const std::string& name, llvm::orc::JITDylib& dyLib) const
{
	return executionSession->lookup({&dyLib}, mangler->operator()(name));
}


llvm::JITTargetAddress LLVMJit::getSymbolAddress(const std::string& name, llvm::orc::JITDylib& dyLib) const
{
	return llvm::cantFail(findSymbol(name, dyLib)).getAddress();
}


/*void LLVMJit::removeModule(llvm::orc::VModuleKey moduleKey)
{
	compileLayer->
	llvm::cantFail(compileLayer->removeModule(moduleKey));
}*/


LLVMJitInitializer::LLVMJitInitializer()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();
}
