/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "LLVMJit.h"
#include "LLVMTypes.h"

#include <iostream>


LLVMJit::LLVMJit():
	context(new llvm::orc::ThreadSafeContext(llvm::make_unique<llvm::LLVMContext>())),
	targetMachineBuilder(llvm::orc::JITTargetMachineBuilder::detectHost().get()),
	targetMachine(std::move(targetMachineBuilder.createTargetMachine().get())),
	executionSession(new llvm::orc::ExecutionSession()),
	dataLayout(new llvm::DataLayout(targetMachineBuilder.getDefaultDataLayoutForTarget().get())),
	mangler(new llvm::orc::MangleAndInterner(*executionSession, *dataLayout)),
	/*symbolResolver(
		llvm::orc::createLegacyLookupResolver(
            [this](const std::string &name) -> llvm::JITSymbol 
			{
				if (auto symbol = compileLayer->findSymbol(name, false))
				{
					return symbol;
				}
				else if (auto error = symbol.takeError())
				{
					return std::move(error);
				}
				if (auto symbolAddress = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name))
				{
					return llvm::JITSymbol(symbolAddress, llvm::JITSymbolFlags::Exported);
				}
				return nullptr;
            },
            [](llvm::Error error) { llvm::cantFail(std::move(error), "lookupFlags failed"); })),*/
	objectLinkLayer(new llvm::orc::RTDyldObjectLinkingLayer(*executionSession,
															[]() {	return llvm::make_unique<llvm::SectionMemoryManager>();})),
	compileLayer(new llvm::orc::IRCompileLayer(*executionSession.get(), *(objectLinkLayer.get()), llvm::orc::ConcurrentIRCompiler(targetMachineBuilder)))
{
	objectLinkLayer->setAutoClaimResponsibilityForObjectSymbols(true);
	objectLinkLayer->setOverrideObjectFlagsWithResponsibilityFlags(true);
    executionSession->getMainJITDylib().setGenerator(llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(*dataLayout)));
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

	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::pointerType};
		LLVMTypes::functionRetPtrArgPtr = llvm::FunctionType::get(LLVMTypes::pointerType, parameters, false);
	}

	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::pointerType, LLVMTypes::pointerType};
		LLVMTypes::functionRetPtrArgPtr_Ptr = llvm::FunctionType::get(LLVMTypes::pointerType, parameters, false);
	}

	{
		std::vector<llvm::Type*> parameters = {LLVMTypes::pointerType, LLVMTypes::intType};
		LLVMTypes::functionRetPtrArgPtr_Int = llvm::FunctionType::get(LLVMTypes::pointerType, parameters, false);
	}
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


void LLVMJit::addModule(std::unique_ptr<llvm::Module>& module)
{
    // Add the module to the JIT with a new VModuleKey.
	llvm::cantFail(compileLayer->add(executionSession->getMainJITDylib(), llvm::orc::ThreadSafeModule(std::move(module), *context.get())));
}


llvm::Expected<llvm::JITEvaluatedSymbol> LLVMJit::findSymbol(const std::string& name) const
{
	return executionSession->lookup({&executionSession->getMainJITDylib()}, mangler->operator()(name));
}


llvm::JITTargetAddress LLVMJit::getSymbolAddress(const std::string& name) const
{
	return findSymbol(name).get().getAddress();
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
	llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}
