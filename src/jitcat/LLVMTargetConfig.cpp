/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2021
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Configuration.h"
#include "jitcat/LLVMTargetConfig.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/LLVMTargetConfigOptions.h"

#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>


using namespace jitcat;
using namespace jitcat::LLVM;


LLVMTargetConfig::LLVMTargetConfig(bool isJITTarget, bool sretBeforeThis, bool useThisCall, bool callerDestroysTemporaryArguments, 
								   bool enableSymbolSearchWorkaround, bool is64BitTarget, unsigned int sizeOfBoolInBits, 
								   std::string objectFileExtension, std::unique_ptr<LLVMTargetConfigOptions> llvmOptions_):
	isJITTarget(isJITTarget),
	is64BitTarget(is64BitTarget),
	sretBeforeThis(sretBeforeThis),
	useThisCall(useThisCall),
	callerDestroysTemporaryArguments(callerDestroysTemporaryArguments),
	enableSymbolSearchWorkaround(enableSymbolSearchWorkaround),
	sizeOfBoolInBits(sizeOfBoolInBits),
	objectFileExtension(objectFileExtension),
	llvmOptions(std::move(llvmOptions_))
{
	llvmOptions->llvmTypes = std::make_unique<LLVMTypes>(is64BitTarget, sizeOfBoolInBits);
	std::string errorMessage;
	const llvm::Target* target = llvm::TargetRegistry::lookupTarget(llvmOptions->targetTripple, errorMessage);
	if (target != nullptr)
	{
		llvmOptions->targetMachine.reset(target->createTargetMachine(llvmOptions->targetTripple, llvmOptions->cpuName, llvmOptions->subtargetFeatures.getString(), llvmOptions->targetOptions, 
										llvmOptions->relocationModel, llvmOptions->codeModel, llvmOptions->optimizationLevel, isJITTarget));
		llvmOptions->dataLayout = std::make_unique<llvm::DataLayout>(llvmOptions->targetMachine->createDataLayout());
	}
	if (isJITTarget)
	{
		llvmOptions->targetMachineBuilder = llvm::cantFail(llvm::orc::JITTargetMachineBuilder::detectHost());
	}
}


LLVMTargetConfig::~LLVMTargetConfig()
{
}


std::unique_ptr<LLVMTargetConfig> LLVMTargetConfig::createJITTargetConfig()
{
	return createTargetConfigForCurrentMachine(true);
}


std::unique_ptr<LLVMTargetConfig> LLVMTargetConfig::createConfigForPreconfiguredTarget(LLVMTarget target)
{
	switch (target)
	{
		case LLVMTarget::CurrentMachine:		return createTargetConfigForCurrentMachine(false);
		case LLVMTarget::CurrentMachineJIT:		return createTargetConfigForCurrentMachine(true);
		case LLVMTarget::Windows_X64:			return createGenericWindowsx64Target();
		case LLVMTarget::Playstation4:			return createPS4Target();
		case LLVMTarget::XboxOne:				return createXboxOneTarget();
	}
	return nullptr;
}


llvm::TargetMachine& LLVMTargetConfig::getTargetMachine() const
{
	return *llvmOptions->targetMachine.get();
}


const llvm::DataLayout& LLVMTargetConfig::getDataLayout() const
{
	return *llvmOptions->dataLayout.get();
}


llvm::orc::JITTargetMachineBuilder* LLVMTargetConfig::getTargetMachineBuilder() const
{
	if (llvmOptions->targetMachineBuilder.has_value())
	{
		return &llvmOptions->targetMachineBuilder.value();
	}
	else
	{
		return nullptr;
	}
}


const LLVMTypes& LLVMTargetConfig::getLLVMTypes() const
{
	return *llvmOptions->llvmTypes.get();
}


const LLVMTargetConfigOptions& LLVMTargetConfig::getOptions() const
{
	return *llvmOptions.get();
}


std::unique_ptr<LLVMTargetConfig> LLVMTargetConfig::createTargetConfigForCurrentMachine(bool isJITTarget)
{
	constexpr bool isWin32 = 
	#ifdef WIN32
			true;
	#else
			false;
	#endif

	std::unique_ptr<LLVMTargetConfigOptions> llvmOptions = std::make_unique<LLVMTargetConfigOptions>();

	bool sretBeforeThis = !isWin32;
	bool callerDestroysTemporaryArguments =  !isWin32;
	bool useThisCall = isWin32;
	bool enableSymbolSearchWorkaround = isWin32;
	
	#ifdef _WIN64
		llvmOptions->defaultLLVMCallingConvention = llvm::CallingConv::Win64;
	#else
		llvmOptions->defaultLLVMCallingConvention = llvm::CallingConv::C;
	#endif

	const char* objectFileExtension = "o";
	if (isWin32)
	{
		objectFileExtension = "obj";
	}
	llvmOptions->targetTripple = llvm::sys::getProcessTriple();
	llvmOptions->cpuName = llvm::sys::getHostCPUName();


	llvm::StringMap<bool> featureMap;
	llvm::sys::getHostCPUFeatures(featureMap);
	for (auto &Feature : featureMap)
	{
		llvmOptions->subtargetFeatures.AddFeature(Feature.first(), Feature.second);
	}

	if (isJITTarget)
	{
		llvmOptions->targetOptions.EmulatedTLS = true;
		llvmOptions->targetOptions.ExplicitEmulatedTLS = true;
	}

	llvmOptions->optimizationLevel = llvm::CodeGenOpt::Level::Default;
	
	return std::make_unique<LLVMTargetConfig>(isJITTarget, sretBeforeThis, useThisCall, callerDestroysTemporaryArguments, 
											  enableSymbolSearchWorkaround, sizeof(uintptr_t) == 8, (unsigned int)sizeof(bool) * 8,
											  objectFileExtension, std::move(llvmOptions));	
}


std::unique_ptr<LLVMTargetConfig> LLVMTargetConfig::createGenericWindowsx64Target()
{
	constexpr bool isWin32 = true;
	std::unique_ptr<LLVMTargetConfigOptions> llvmOptions = std::make_unique<LLVMTargetConfigOptions>();

	bool sretBeforeThis = !isWin32;
	bool callerDestroysTemporaryArguments =  !isWin32;
	bool useThisCall = isWin32;
	bool enableSymbolSearchWorkaround = isWin32;
	
	llvmOptions->targetTripple = "x86_64-pc-windows-msvc";
	llvmOptions->cpuName = "x86-64";
	llvmOptions->defaultLLVMCallingConvention = llvm::CallingConv::Win64;

	return std::make_unique<LLVMTargetConfig>(false, sretBeforeThis, useThisCall, callerDestroysTemporaryArguments, 
											  enableSymbolSearchWorkaround, true, 8,
											  "obj", std::move(llvmOptions));		
}


std::unique_ptr<LLVMTargetConfig> LLVMTargetConfig::createXboxOneTarget()
{
	constexpr bool isWin32 = true;
	std::unique_ptr<LLVMTargetConfigOptions> llvmOptions = std::make_unique<LLVMTargetConfigOptions>();

	bool sretBeforeThis = !isWin32;
	bool callerDestroysTemporaryArguments =  !isWin32;
	bool useThisCall = isWin32;
	bool enableSymbolSearchWorkaround = isWin32;
	
	llvmOptions->targetTripple = "x86_64-pc-win32";
	llvmOptions->cpuName = "btver2";
	llvmOptions->defaultLLVMCallingConvention = llvm::CallingConv::X86_FastCall;

	llvmOptions->targetOptions.UnsafeFPMath = true;
	llvmOptions->targetOptions.NoSignedZerosFPMath = true;
	llvmOptions->targetOptions.ThreadModel = llvm::ThreadModel::POSIX;
	llvmOptions->targetOptions.DebuggerTuning = llvm::DebuggerKind::Default;
	llvmOptions->targetOptions.DataSections = true;
	llvmOptions->codeModel = llvm::CodeModel::Small;
	llvmOptions->relocationModel = llvm::Reloc::Model::PIC_;

	return std::make_unique<LLVMTargetConfig>(false, sretBeforeThis, useThisCall, callerDestroysTemporaryArguments, 
											  enableSymbolSearchWorkaround, true, 8,
											  "obj", std::move(llvmOptions));	

}


std::unique_ptr<LLVMTargetConfig> LLVMTargetConfig::createPS4Target()
{
	std::unique_ptr<LLVMTargetConfigOptions> llvmOptions = std::make_unique<LLVMTargetConfigOptions>();

	bool sretBeforeThis = true;
	bool callerDestroysTemporaryArguments = true;
	bool useThisCall = false;
	bool enableSymbolSearchWorkaround = false;

	llvmOptions->targetTripple = "x86_64-scei-ps4";
	llvmOptions->cpuName = "btver2";
	llvmOptions->defaultLLVMCallingConvention  = llvm::CallingConv::X86_64_SysV;
	llvmOptions->targetOptions.RelaxELFRelocations = true;
	llvmOptions->targetOptions.UnsafeFPMath = true;
	llvmOptions->targetOptions.NoNaNsFPMath = true;
	llvmOptions->targetOptions.NoInfsFPMath = true;
	llvmOptions->targetOptions.NoTrappingFPMath = false;
	
	llvmOptions->targetOptions.ThreadModel = llvm::ThreadModel::POSIX;
	llvmOptions->targetOptions.DebuggerTuning = llvm::DebuggerKind::SCE;
	llvmOptions->targetOptions.DataSections = true;
	llvmOptions->codeModel = llvm::CodeModel::Small;
	llvmOptions->relocationModel = llvm::Reloc::Model::PIC_;

	llvmOptions->strongStackProtection = false;
	llvmOptions->explicitEnableTailCalls = true;
	llvmOptions->nonLeafFramePointer = true;

	return std::make_unique<LLVMTargetConfig>(false, sretBeforeThis, useThisCall, callerDestroysTemporaryArguments, 
											  enableSymbolSearchWorkaround, true, 8, 
											  "o", std::move(llvmOptions));
}
