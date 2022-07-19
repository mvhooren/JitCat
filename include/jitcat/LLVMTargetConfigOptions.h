/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2021
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/LLVMTypes.h"

#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/MC/SubtargetFeature.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Target/TargetOptions.h>
#include <memory>
#include <string>


namespace jitcat::LLVM
{
	struct LLVMTargetConfigOptions
	{
		unsigned int defaultLLVMCallingConvention = llvm::CallingConv::C;
		
		std::unique_ptr<LLVMTypes> llvmTypes;
		//The target tripple that identifies CPU architecture, OS and compiler compatibility
		std::string targetTripple;
		//The name of the target CPU
		std::string cpuName;


		//Contains all the target specific information for the machine that we are compiling for. Among other things, the target CPU type.
		std::unique_ptr<llvm::TargetMachine> targetMachine;
		//Specifies the layout of structs and the type of name mangling used based on the target machine as well as endianness.
		std::unique_ptr<const llvm::DataLayout> dataLayout;

		bool strongStackProtection = false;
		bool explicitEnableTailCalls = false;
		bool nonLeafFramePointer = false;

		//Target specific options
		llvm::TargetOptions targetOptions;
		llvm::SubtargetFeatures subtargetFeatures;
		llvm::CodeGenOpt::Level optimizationLevel = llvm::CodeGenOpt::Default;
		llvm::Optional<llvm::Reloc::Model> relocationModel;
		llvm::Optional<llvm::CodeModel::Model> codeModel;

		std::optional<llvm::orc::JITTargetMachineBuilder> targetMachineBuilder;
	};
}
