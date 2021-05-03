/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2021
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Configuration.h"
#include "jitcat/LLVMTargetConfig.h"
#include "jitcat/LLVMTypes.h"

#include <llvm/IR/CallingConv.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>

using namespace jitcat;
using namespace jitcat::LLVM;


LLVMTargetConfig::LLVMTargetConfig(bool isJITTarget, bool sretBeforeThis, bool useThisCall, bool callerDestroysTemporaryArguments, 
								   bool enableSymbolSearchWorkaround, bool is64BitTarget, unsigned int sizeOfBoolInBits, 
								   unsigned int defaultLLVMCallingConvention, const std::string& targetTripple, const std::string& cpuName,
								   const llvm::TargetOptions& targetOptions, const llvm::SubtargetFeatures& subtargetFeatures, 
								   llvm::CodeGenOpt::Level optimizationLevel, llvm::Optional<llvm::Reloc::Model> relocationModel,
								   llvm::Optional<llvm::CodeModel::Model> codeModel):
	isJITTarget(isJITTarget),
	is64BitTarget(is64BitTarget),
	sretBeforeThis(sretBeforeThis),
	useThisCall(useThisCall),
	callerDestroysTemporaryArguments(callerDestroysTemporaryArguments),
	enableSymbolSearchWorkaround(enableSymbolSearchWorkaround),
	sizeOfBoolInBits(sizeOfBoolInBits),
	defaultLLVMCallingConvention(defaultLLVMCallingConvention),
	llvmTypes(std::make_unique<LLVMTypes>(is64BitTarget, sizeOfBoolInBits)),
	targetTripple(targetTripple),
	cpuName(cpuName),
	targetOptions(targetOptions),
	subtargetFeatures(subtargetFeatures),
	optimizationLevel(optimizationLevel),
	relocationModel(relocationModel),
	codeModel(codeModel)
{
	std::string errorMessage;
	const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTripple, errorMessage);
	if (target != nullptr)
	{
		targetMachine.reset(target->createTargetMachine(targetTripple, cpuName, subtargetFeatures.getString(), targetOptions, 
													   relocationModel, codeModel, optimizationLevel, isJITTarget));
		dataLayout = std::make_unique<llvm::DataLayout>(targetMachine->createDataLayout());
	}
	if (isJITTarget)
	{
		targetMachineBuilder = llvm::cantFail(llvm::orc::JITTargetMachineBuilder::detectHost());
	}
}


LLVMTargetConfig::~LLVMTargetConfig()
{
}


std::unique_ptr<LLVMTargetConfig> LLVMTargetConfig::createJITTargetConfig()
{
	constexpr bool isWin32 = 
#ifdef WIN32
		true;
#else
		false;
#endif

	bool sretBeforeThis = !isWin32;
	bool callerDestroysTemporaryArguments =  !isWin32;
	bool useThisCall = isWin32;
	bool enableSymbolSearchWorkaround = isWin32;
	
	unsigned int defaultCallingConvention = llvm::CallingConv::C;
	#ifdef _WIN64
		defaultCallingConvention = llvm::CallingConv::Win64;
	#endif


	std::string targetTripple = llvm::sys::getProcessTriple();
	std::string cpuName = llvm::sys::getHostCPUName();

	llvm::SubtargetFeatures features;

	llvm::StringMap<bool> featureMap;
	llvm::sys::getHostCPUFeatures(featureMap);
	for (auto &Feature : featureMap)
	{
		features.AddFeature(Feature.first(), Feature.second);
	}

	llvm::TargetOptions options;
	options.EmulatedTLS = true;
	options.ExplicitEmulatedTLS = true;

	return std::make_unique<LLVMTargetConfig>(true, sretBeforeThis, useThisCall, callerDestroysTemporaryArguments, 
											  enableSymbolSearchWorkaround, sizeof(uintptr_t) == 8, (unsigned int)sizeof(bool) * 8,
											  defaultCallingConvention, targetTripple, cpuName, 
											  options, features, llvm::CodeGenOpt::Level::Default);
}


llvm::TargetMachine& LLVMTargetConfig::getTargetMachine() const
{
	return *targetMachine.get();
}


const llvm::DataLayout& LLVMTargetConfig::getDataLayout() const
{
	return *dataLayout.get();
}


llvm::Expected<const llvm::orc::JITTargetMachineBuilder&> LLVMTargetConfig::getTargetMachineBuilder() const
{
	if (targetMachineBuilder.hasValue())
	{
		return targetMachineBuilder.getValue();
	}
	else
	{
		return llvm::make_error<llvm::StringError>("Not a JIT target configuration.", llvm::inconvertibleErrorCode());
	}
}


const LLVMTypes& LLVMTargetConfig::getLLVMTypes() const
{
	return *llvmTypes.get();
}
